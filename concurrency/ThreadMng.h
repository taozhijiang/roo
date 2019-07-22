/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_CONCURRENCY_THREAD_MNG_H__
#define __ROO_CONCURRENCY_THREAD_MNG_H__


#include <xtra_rhel.h>

#include <thread>

#include <other/Log.h>
#include <container/EQueue.h>
#include <system/ConstructException.h>


// 这是一个不带任务缓冲的线程管理类，添加进来的任务
// 都会创建新的线程来异步执行

namespace roo {
    


// 可执行任务类型
typedef std::function<int()> TaskRunnable;

class ThreadGuard;
class ThreadMng {

    // 禁止拷贝
    __noncopyable__(ThreadMng)

public:
    explicit ThreadMng(uint8_t max_spawn) :
        max_spawn_(max_spawn) {
    }

    ~ThreadMng() {
        join_all();
    }

    // 增加任务执行线程，但是不会超过限制数目
    bool try_add_task(const TaskRunnable& run);

    // 阻塞，直到添加完成
    bool add_task(const TaskRunnable& run);

    // 阻塞直到所有任务都join
    void join_all();


private:

    friend class ThreadGuard;
    
    bool add_thread_guard(ThreadGuard* thd);
    bool del_thread_guard(ThreadGuard* thd);

private:

    // 最大并发线程数，0表示没有限制 - 危险
    const uint32_t max_spawn_;

    std::mutex lock_;
    std::condition_variable item_notify_;
    
    std::set<ThreadGuard*> threads_;
};


class ThreadGuard {

    __noncopyable__(ThreadGuard)

public:

    // TODO: create failed.
    explicit ThreadGuard(ThreadMng& mng, const TaskRunnable& run) :
        mng_(mng),
        func_(run) {
        thread_.reset(new std::thread(std::bind(&ThreadGuard::internal_run, this)));
        mng_.add_thread_guard(this);
    }

    // 当ThreadMng在vector中的智能指针删除的时候，会触发这边
    // 自动将线程从thread_group中移除掉
    ~ThreadGuard() {
        join();
        log_info("thread exit ...");
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

    ThreadMng& mng_;
    const TaskRunnable func_;
    std::unique_ptr<std::thread> thread_;
};



} // roo

#endif // __ROO_CONCURRENCY_THREAD_MNG_H__
