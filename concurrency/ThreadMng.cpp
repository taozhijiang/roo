/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#include "ThreadMng.h"

namespace roo {

bool ThreadMng::try_add_task(const TaskRunnable& run) {

    std::lock_guard<std::mutex> lock(lock_);
    if (max_spawn_ && threads_.size() >= max_spawn_)
        return false;

    ThreadGuard* task = new ThreadGuard(*this, run);
    if (task) {
        threads_.insert(task);
        return true;
    }
    
    log_err("Create ThreadGuard failed.");
    return false;
}


bool ThreadMng::add_task(const TaskRunnable& run) {

    std::unique_lock<std::mutex> lock(lock_);
    while (max_spawn_ && threads_.size() >= max_spawn_) {
        item_notify_.wait(lock);
    }

    // impossible actually
    if (max_spawn_ && threads_.size() >= max_spawn_)
        PANIC("Impossible idle threads check.");

    ThreadGuard* task = new ThreadGuard(*this, run);
    if (task) {
        threads_.insert(task);
        return true;
    }
    
    log_err("Create ThreadGuard failed.");
    return false;
}

// 阻塞直到所有任务都join
void ThreadMng::join_all() {

    std::unique_lock<std::mutex> lock(lock_);
    for (auto it = threads_.begin(); it != threads_.end(); ++it) {
        (*it)->join();
        delete (*it);
    }
    
    log_warning("Join all threads finished, count %ld.", threads_.size());
    
    threads_.clear();
    item_notify_.notify_all();
}

bool ThreadMng::add_thread_guard(ThreadGuard* thd) {

    std::unique_lock<std::mutex> lock(lock_);
    if (threads_.find(thd) != threads_.end()) {
        log_err("add thread already in set: %p", thd);
        return false;
    }

    threads_.insert(thd);
    return true;
}

bool ThreadMng::del_thread_guard(ThreadGuard* thd) {

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


} // roo
