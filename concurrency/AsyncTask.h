/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_CONCURRENCY_ASYNC_TASK_H__
#define __ROO_CONCURRENCY_ASYNC_TASK_H__


#include <xtra_rhel.h>

// 本来想直接使用future来做的，但是现有生产环境太旧，std::future
// 和boost::future都不可用，这里模拟一个任务结构，采用one-task-per-thread的方式，
// 进行弹性的任务创建执行。

#include <memory>
#include <functional>

#include <other/Log.h>
#include <system/ConstructException.h>
#include <concurrency/ThreadMng.h>
#include <container/EQueue.h>

namespace roo {

// 简单对ThreadMng的封装，用于一个任务一个task，不过带了一个
// EQueue缓冲任务
class AsyncTask {

    // 禁止拷贝
    AsyncTask(const AsyncTask&) = delete;
    AsyncTask& operator=(const AsyncTask&) = delete;

public:

    explicit AsyncTask(uint8_t max_spawn) :
        max_spawn_(max_spawn),
        thread_run_(),
        thread_terminate_(false),
        thread_mng_(max_spawn_),
        tasks_() {

        thread_run_.reset(new std::thread(std::bind(&AsyncTask::run, this)));
        if (!thread_run_) {
            log_err("create run work thread failed! ");
            throw ConstructException("AsyncTask Thread Create Fail");
        }
    }

    void terminate() {

        thread_terminate_ = true;
        if (thread_run_ && thread_run_->joinable())
            thread_run_->join();
    }

    ~AsyncTask() {
        terminate();
        log_warning("AsyncTask destroy successfully.");
    }

    void add_async_task(const TaskRunnable& func) {
        tasks_.PUSH(func);
    }


private:

    void run() {

        log_warning("AsyncTask thread %#lx begin to run ...", (long)pthread_self());

        while (true) {

            if (thread_terminate_) {
                log_warning("AsyncTask thread %#lx about to terminate ...", (long)pthread_self());
                break;
            }

            std::vector<TaskRunnable> tasks{};
            size_t count = tasks_.POP(tasks, max_spawn_, 1000 /*1s*/);
            if (!count) {  // 空闲
                continue;
            }

            for (size_t i = 0; i < tasks.size(); ++i) {
                thread_mng_.add_task(tasks[i]);
            }

            log_warning("count %d task process done!", static_cast<int>(tasks.size()));
        }
    }

private:

    uint32_t max_spawn_;
    std::unique_ptr<std::thread> thread_run_;
    bool thread_terminate_;

    // 封装线程管理细节
    ThreadMng thread_mng_;

    // 待单独执行的任务列表
    EQueue<TaskRunnable> tasks_;
};


} // roo

#endif // __ROO_CONCURRENCY_ASYNC_TASK_H__
