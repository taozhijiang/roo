/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_CONTAINER_LRU_CACHE_H__
#define __ROO_CONTAINER_LRU_CACHE_H__

#include <string>
#include <vector>
#include <unordered_map>

#include <assert.h>

// 基于unordered_map(hash)实现的LRU缓存数据结构，可以集成与应用软件内部的缓存机制
// 1. (TODO) 更加准确的内存使用计算，因为某些类型(至少std::string)的存储数据是放在内存中的
// 2. (TODO) 提供元素TTL过期自动删除的机制
//
// 除了内部保护list外，数据结构本身不持锁，使用者进行并发控制的保护

namespace roo {

    
//
// 辅助类

// 简单的链表，用来组织数据结构
template<typename TKey>
struct ListNode {
    
    explicit ListNode(): 
        key_(),
        prev_(NULL), 
        next_(NULL) {}

    explicit ListNode(const TKey& key) :
        key_(key), 
        prev_(NULL), 
        next_(NULL) {}

    // 可以存储元素的迭代器地址，但是当rehash的时候会导致失效
    TKey      key_; 
    ListNode* prev_;
    ListNode* next_;
};

template<typename TKey, typename TValue>
struct ValueNode {

    typedef ListNode<TKey> ListNodeType;

    ValueNode():
        node_(NULL) {}

    ValueNode(const TValue& value, ListNodeType* node): 
        value_(value), 
        node_(node) {}

    TValue        value_;
    ListNodeType* node_;
};


template<typename TKey, typename TValue>
class LruCache {

public:
    typedef TKey   key_type;
    typedef TValue value_type;

    typedef ListNode<TKey> ListNodeType;
    typedef ValueNode<TKey, TValue> ValueNodeType;

    typedef std::unordered_map<TKey, ValueNodeType> Container;
    typedef std::pair<const TKey, TValue>  SnapshotValue;


    LruCache(size_t max_count, size_t max_memory = 0):
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
        
    ~LruCache() {
        clear();
    }

    // 禁止拷贝
    LruCache(const LruCache& other) = delete;
    LruCache& operator=(const LruCache&) = delete;

    
public:
    
    bool find(const TKey& key, TValue& val) {

        auto iter = container_.find(key);
        if(iter == container_.end())
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
        if(!ret.second) {
            delete node;
            return false;
        }

        link_push_front(node);
        mem_used_ = mem_used_ + sizeof(key) + sizeof(valNode) + sizeof(ListNodeType);

        evict();
        return true;
    }

    bool insert_or_update(const TKey& key, const TValue& value) {
    
        auto iter = container_.find(key);
        if(iter == container_.end())
            return insert(key, value);

        // update
        ValueNodeType& oldValue = iter->second;
        size_t old_value_size = sizeof(oldValue.value_);
        oldValue.value_ = value;

        delink(oldValue.node_);
        link_push_front(oldValue.node_);
        mem_used_ = mem_used_ + sizeof(value) - old_value_size;

        evict();
        return true;
    }

    // 清空整个缓存
    void clear() {

        container_.clear();
        ListNodeType* node = head_.next_;
        ListNodeType* next;
        while(node != &tail_) {
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
    void snapshot_keys(std::vector<TKey>& keys) {

        keys.reserve(keys.size() + container_.size());
        for(ListNodeType* node = head_.next_; node != &tail_; node=node->next_) {
            keys.push_back(node->key_);
        }
    }


    size_t item_count() const {
        return container_.size();
    }

    size_t item_mem_used() const {
        return mem_used_;
    }


private:

    size_t item_mem_size(ValueNodeType& valueNode) const {
        return 0;
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

        while(container_.size() >= max_count_ ||
              (max_memory_ != 0 && mem_used_ >= max_memory_) ) {

            if(container_.size() <= 1)
                return;
            
            ListNodeType* ill = tail_.prev_;
            if(ill == &head_)
                return;
            
            auto iter = container_.find(ill->key_);
            assert(iter != container_.end());

            mem_used_ = mem_used_ - ( sizeof(iter->first) + sizeof(iter->second) + sizeof(ListNodeType) );
            delink(ill);
            container_.erase(ill->key_);
            delete ill;
        }
    }

    const size_t max_count_;      // 最大元素个数
    const size_t max_memory_;     // 最大内存占用量(估算)
    size_t       mem_used_;

    Container    container_;   // 主元素存储hash容器
    
    ListNodeType head_;         // 实体非指针，用于队列的头尾
    ListNodeType tail_;
};



} // roo

#endif // __ROO_CONTAINER_LRU_CACHE_H__

