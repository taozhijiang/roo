/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_CONCURRENCY_THREAD_MNG_H__
#define __ROO_CONCURRENCY_THREAD_MNG_H__


#include <xtra_rhel.h>

#include <container/EQueue.h>

namespace roo {

// 可执行任务类型
typedef std::function<int()> TaskRunnable;

class ThreadGuard {

    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;

public:

    // TODO: create failed.
    explicit ThreadGuard(ThreadMng& mng, TaskRunnable& run) :
        mng_(mng),
        func_(run) {
        thread_ = new boost::thread(internal_run);
        mng_.add_thread_guard(this);
    }

    // 当ThreadMng在vector中的智能指针删除的时候，会触发这边
    // 自动将线程从thread_group中移除掉
    ~ThreadGuard() {
        join();
        log_info("thread exit...");
    }

    void join() {
        if (thread_ && thread_->joinable())
            thread_->join();
    }


private:

    void internal_run() {
        if (func_) {
            int code = func_();
            if (code != 0)
                log_err("ThreadGuard task run return %d.", code);
        }

        // task finished hook
        mng_.del_thread_guard(this);
    }

    ThreadMng&     mng_;
    TaskRunnable   func_;
    std::unique_ptr<std::thread> thread_;
};


// 一个不带任务缓冲的线程管理类
class ThreadMng {

    // 禁止拷贝
    ThreadMng(const ThreadMng&) = delete;
    ThreadMng& operator=(const ThreadMng&) = delete;

public:
    explicit ThreadMng(uint8_t max_spawn) :
        max_spawn_(max_spawn) {
        if (max_spawn_ == 0)
            max_spawn_ = 1;
    }

    ~ThreadMng() {
        join_all();
    }

    // 增加任务执行线程，但是不会超过限制数目
    bool try_add_task(TaskRunnable& run) {

        std::lock_guard<std::mutex> lock(lock_);
        if (threads_.size() >= max_spawn_)
            return false;

        ThreadGuard* task = new ThreadGuard(*this, run);
        if (task)
            threads_.emplace_back(task);
        return true;
    }

    // 阻塞，直到添加完成
    bool add_task(TaskRunnable& run) {

        std::unique_lock<std::mutex> lock(lock_);
        while (threads_.size() >= max_task_) {
            item_notify_.wait(lock);
        }

        // impossible actually
        if (threads_.size() >= max_task_)
            return false;

        ThreadGuard* task = new ThreadGuard(*this, run);
        if (task)
            threads_.emplace_back(task);
        return true;
    }

    // 阻塞直到所有任务都join
    void join_all() {

        std::unique_lock<std::mutex> lock(lock_);
        for (auto it = threads_.begin(); it != threads_.end(); ++it) {
            (*it)->join();
            delete (*it);
        }

        threads_.clear();
        item_notify_.notify_all();
    }

    bool add_thread_guard(ThreadGuard* thd) {

        std::unique_lock<std::mutex> lock(lock_);
        if (threads_.find(thd) != threads_.end()) {
            log_err("add thread already in set: %p", thd);
            return false;
        }

        threads_.insert(thd);
        return true;
    }

    bool del_thread_guard(ThreadGuard* thd) {

        std::unique_lock<std::mutex> lock(lock_);
        if (threads_.find(thd) == threads_.end()) {
            log_err("del thread not found in set: %p", thd);
            return false;
        }

        threads_.erase(thd);
        delete thd;
        item_notify_.notify_all();
        return true;
    }


private:

    uint32_t max_spawn_;

    std::mutex lock_;
    std::set<ThreadGuard*> threads_;
};



} // roo

#endif // __ROO_CONCURRENCY_THREAD_MNG_H__
