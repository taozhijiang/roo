/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_CONTAINER_LRU_CACHE_MEM_H__
#define __ROO_CONTAINER_LRU_CACHE_MEM_H__

#include <string>
#include <vector>
#include <unordered_map>

#include <assert.h>

#include "LruCache.h"

// 基于unordered_map(hash)实现的LRU缓存数据结构，可以集成与应用软件内部的缓存机制
// 1. (TODO) 更加准确的内存使用计算，因为某些类型(至少std::string)的存储数据是放在内存中的
// 2. (TODO) 提供元素TTL过期自动删除的机制
//
// 除了内部保护list外，数据结构本身不持锁，使用者进行并发控制的保护

namespace roo {


// 每个元素插入后需要额外的内部管理空间
const static size_t kSizeAdditional =
    sizeof(ValueNode<int32_t, int32_t>) + sizeof(ListNode<int32_t>) - sizeof(int32_t) - sizeof(int32_t);

// 默认的空间占用量计算，对于某些类型返回的类型长度可能不准确
template<typename T>
struct SizeOf : std::unary_function<T, size_t> {
    size_t operator ()(const T& t) const {
        return sizeof(t);
    }
};

// 特例化常用的std::string类型的参数
// sizeof(std::string)默认返回是8(64位机器)，然后再加上实际的数据使用量
template<>
size_t SizeOf<std::string>::operator()(const std::string& t) const {
    // 觉得使用capacity可能更准确一些???
    //return sizeof(std::string) + t.capacity();

    return sizeof(std::string) + t.size();
}

template<typename TKey, typename TValue,
         class ValueSize = SizeOf<TValue>,
         class KeySize   = SizeOf<TKey> >
class LruCacheMem {

public:
    typedef TKey   key_type;
    typedef TValue value_type;

    typedef ListNode<TKey> ListNodeType;
    typedef ValueNode<TKey, TValue> ValueNodeType;

    typedef std::unordered_map<TKey, ValueNodeType> Container;
    typedef std::pair<const TKey, TValue>  SnapshotValue;


    LruCacheMem(size_t max_count, size_t max_memory = 0) :
        max_count_(max_count),
        max_memory_(max_memory),
        mem_used_(0),
        head_(),
        tail_() {
        head_.prev_ = NULL;
        head_.next_ = &tail_;
        tail_.prev_ = &head_;
        tail_.next_ = NULL;
    }

    ~LruCacheMem() {
        clear();
    }

    // 禁止拷贝
    LruCacheMem(const LruCacheMem& other) = delete;
    LruCacheMem& operator=(const LruCacheMem&) = delete;


public:

    bool find(const TKey& key, TValue& val) {

        auto iter = container_.find(key);
        if (iter == container_.end())
            return false;

        ListNodeType* node = iter->second.node_;
        val = iter->second.value_;
        delink(node);
        link_push_front(node);
        return true;
    }


    // 不更新LRU列表
    bool find(const TKey& key) const {
        return container_.find(key) != container_.end();
    }


    // 成功插入返回true，否则返回false
    bool insert(const TKey& key, const TValue& value) {

        ListNodeType* node = new ListNodeType(key);
        ValueNodeType valNode(value, node);

        std::pair<typename Container::iterator, bool> ret = container_.insert(
            typename Container::value_type(key, valNode)
            );

        // 是否插入
        if (!ret.second) {
            delete node;
            return false;
        }

        link_push_front(node);
        mem_used_ = mem_used_ + calc_item_size(key, value);

        evict();
        return true;
    }

    bool insert_or_update(const TKey& key, const TValue& value) {

        auto iter = container_.find(key);
        if (iter == container_.end())
            return insert(key, value);

        // update
        ValueNodeType& oldValue = iter->second;
        size_t old_value_size = calc_value_size_(oldValue.value_);
        oldValue.value_ = value;

        delink(oldValue.node_);
        link_push_front(oldValue.node_);
        mem_used_ = mem_used_ + calc_value_size_(value) - old_value_size;

        evict();
        return true;
    }

    // 清空整个缓存
    void clear() {

        container_.clear();
        ListNodeType* node = head_.next_;
        ListNodeType* next;
        while (node != &tail_) {
            next = node->next_;
            delete node;
            node = next;
        }

        // reset all
        head_.next_ = &tail_;
        tail_.prev_ = &head_;
        mem_used_ = 0;
    }

    // 对容器中的所有key进行快照并拷贝到keys中
    // 按照MRU->LRU的顺序
    void snapshot_keys(std::vector<TKey>& keys) const {

        keys.reserve(keys.size() + container_.size());
        for(ListNodeType* node = head_.next_;
            node != &tail_;
            node = node->next_){
            keys.push_back(node->key_);
        }
    }

    size_t total_count() const {
        return container_.size();
    }

    size_t total_mem_used() const {
        return mem_used_;
    }

private:

    size_t calc_item_size(const TKey& key, const TValue& val) const {
        return  calc_key_size_(key) * 2 +
               calc_value_size_(val) +
               kSizeAdditional;
    }

    // 列表操作，从LRU链表中删除该节点
    void delink(ListNodeType* node) {
        ListNodeType* prev_ = node->prev_;
        ListNodeType* next_ = node->next_;
        prev_->next_ = node->next_;
        next_->prev_ = node->prev_;
        node->next_ = node->prev_ = NULL;
    }

    // 在LRU链表头部增加该节点
    void link_push_front(ListNodeType* node) {
        ListNodeType* old_head = head_.next_;
        node->prev_ = &head_;
        node->next_ = old_head;

        old_head->prev_ = node;
        head_.next_ = node;
    }

    // 淘汰元素
    void evict() {

        while (container_.size() >= max_count_ ||
               (max_memory_ != 0 && mem_used_ >= max_memory_)) {

            if (container_.size() <= 1)
                return;

            ListNodeType* ill = tail_.prev_;
            if (ill == &head_)
                return;

            const auto iter = container_.find(ill->key_);
            assert(iter != container_.cend());

            mem_used_ = mem_used_ - (calc_item_size(iter->first, iter->second.value_) + kSizeAdditional);
            delink(ill);
            container_.erase(ill->key_);
            delete ill;
        }
    }

    const size_t max_count_;      // 最大元素个数

    ValueSize    calc_value_size_;
    KeySize      calc_key_size_;

    const size_t max_memory_;     // 最大内存占用量(估算)
    size_t       mem_used_;

    Container    container_;   // 主元素存储hash容器

    ListNodeType head_;         // 实体非指针，用于队列的头尾
    ListNodeType tail_;
};



} // roo

#endif // __ROO_CONTAINER_LRU_CACHE_MEM_H__

