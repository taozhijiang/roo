/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_CONCURRENCY_DEFER_TASK_H__
#define __ROO_CONCURRENCY_DEFER_TASK_H__


#include <xtra_rhel.h>

#include <memory>
#include <functional>

#include <other/Log.h>
#include <system/ConstructException.h>
#include <container/EQueue.h>

// 相比AsyncTask，这里采用固定线程池执行EQueue任务的方式

namespace roo {

// 可执行任务类型
typedef std::function<int()> TaskRunnable;


class DeferTask {

    // 禁止拷贝
    DeferTask(const DeferTask&) = delete;
    DeferTask& operator=(const DeferTask&) = delete;

public:

    explicit DeferTask(uint8_t thread_num = 1) :
        threads_(),
        thread_terminate_(false),
        tasks_() {

        if (thread_num == 0)
            thread_num = 1;

        threads_.reserve(thread_num);
        for (int i = 0; i < thread_num; ++i) {
            std::thread* thd = new std::thread(std::bind(&DeferTask::thread_run,  this));
            if (!thd) {
                log_err("create DeferTask thread failed.");

                // 清理已经创建的线程
                thread_terminate_ = true;
                for (size_t j = 0; j < threads_.size(); ++j) {
                    if (threads_[j] && threads_[j]->joinable())
                        threads_[j]->join();
                    delete threads_[j];
                }

                throw ConstructException("DeferTask Thread Create Fail");
            }

            threads_.push_back(thd);
        }

        log_warning("create %u threads for DeferTask successfully!", thread_num);
    }

    void terminate() {

        // flag to terminate
        thread_terminate_ = true;

        // join all threads
        for (size_t j = 0; j < threads_.size(); ++j) {
            if (threads_[j] && threads_[j]->joinable())
                threads_[j]->join();
            delete threads_[j];
        }

        threads_.clear();
    }

    ~DeferTask() {
        terminate();
        log_warning("DeferTask destroy successfully.");
    }

    void add_defer_task(const TaskRunnable& func) {
        tasks_.PUSH(func);
    }

private:
    void thread_run() {

        log_warning("DeferTask thread %#lx begin to run ...", (long)pthread_self());

        while (true) {

            if (thread_terminate_) {
                log_warning("log_warning thread %#lx about to terminate ...", (long)pthread_self());
                break;
            }

            TaskRunnable task = tasks_.POP();

            int code = task();
            if (code != 0)
                log_err("DeferTask run code %d.", code);
        }
    }

private:

    std::vector<std::thread*> threads_;
    bool thread_terminate_;

    // 待单独执行的任务列表
    EQueue<TaskRunnable> tasks_;
};


} // roo

#endif // __ROO_CONCURRENCY_DEFER_TASK_H__
