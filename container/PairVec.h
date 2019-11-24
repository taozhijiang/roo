/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_CONTAINER_PAIR_VEC_H__
#define __ROO_CONTAINER_PAIR_VEC_H__

// 不同于map的key value，使用vector保持推入的顺序
// 内部的操作都是持锁的，保证线程安全

#include <vector>
#include <memory>
#include <mutex>

#include <utility>
#include <functional>

namespace roo {

template<typename K, typename V>
class PairVec {
public:
    typedef std::pair<K, V> Entry;
    typedef std::vector<Entry> Container;
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

public:
    PairVec() :
        lock_(new std::mutex()),
        items_() {
    }

    // not support initilized_list
    explicit PairVec(Container& init) :
        items_({ }) {
        for (auto iter = init.begin(); iter != init.end(); ++iter)
            items_.push_back(*iter);
    }

    ~PairVec() = default;

public:

    // so called iterator delegation...
    iterator BEGIN() {
        return items_.begin();
    }

    iterator END() {
        return items_.end();
    }

    const_iterator CBEGIN() const {
        return items_.cbegin();
    }

    const_iterator CEND() const {
        return items_.cend();
    }

    void PUSH_BACK(const K& k, const V& v) {
        std::lock_guard<std::mutex> lock(*lock_);
        items_.push_back({ k, v }); // make_pair can not use - const ref
    }

    void PUSH_BACK(const Entry& entry) {
        std::lock_guard<std::mutex> lock(*lock_);
        items_.push_back(entry);
    }

    bool EXIST(const K& k) const {
        std::lock_guard<std::mutex> lock(*lock_);
        for (size_t idx = 0; idx < items_.size(); ++idx) {
            if (items_[idx].first == k) {
                return true;
            }
        }
        return false;
    }

    bool FIND(const K& k, V& v) const {
        std::lock_guard<std::mutex> lock(*lock_);
        for (size_t idx = 0; idx < items_.size(); ++idx) {
            if (items_[idx].first == k) {
                v = items_[idx].second;
                return true;
            }
        }
        return false;
    }

    bool FIND(const K& k, Entry& entry) const {
        std::lock_guard<std::mutex> lock(*lock_);
        for (size_t idx = 0; idx < items_.size(); ++idx) {
            if (items_[idx].first == k) {
                entry = items_[idx];
                return true;
            }
        }
        return false;
    }

    V VALUE(const K& k) const {
        std::lock_guard<std::mutex> lock(*lock_);

        V v{}; // default value
        for (size_t idx = 0; idx < items_.size(); ++idx) {
            if (items_[idx].first == k) {
                return items_[idx].second;
            }
        }
        return v;
    }

    size_t SIZE() const {
        std::lock_guard<std::mutex> lock(*lock_);
        return items_.size();
    }

    bool EMPTY() const {
        std::lock_guard<std::mutex> lock(*lock_);
        return items_.empty();
    }

    void CLEAR() {
        std::lock_guard<std::mutex> lock(*lock_);
        items_.clear();
    }

    std::mutex& LOCK() {
        return *lock_;
    }

    // 简单的json序列化，暂时不引入json库
    std::string SERIALIZE() const {
        std::lock_guard<std::mutex> lock(*lock_);

        std::stringstream ss;
        ss << "{";
        for (size_t idx = 0; idx < items_.size(); ++idx) {
            if (idx != 0) {
                ss << ",";
            }
            ss << "\"" << items_[idx].first << "\"" << ":";
            ss << "\"" << items_[idx].second << "\"";
        }
        ss << "}";

        return ss.str();
    }

private:
    std::unique_ptr<std::mutex>  lock_;
    std::vector<std::pair<K, V>> items_;
};

} // end namespace roo

#endif // __ROO_CONTAINER_PAIR_VEC_H__

