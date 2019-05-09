/*-
 * Copyright (c) 2018-2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_IO_SERVICE_H__
#define __ROO_IO_SERVICE_H__

#include <boost/asio.hpp>

#include <thread>
#include <mutex>
#include <memory>

#include <other/Log.h>

// 提供定时回调接口服务


namespace roo {

class IoService {

public:
    IoService():
        lock_(),
        initialized_(false),
        io_service_thread_(),
        io_service_ptr_(new boost::asio::io_service()),
        work_guard_(){
    }

    ~IoService() {
        
        if(io_service_ptr_)
            io_service_ptr_->stop();
        
        work_guard_.reset();
        
        if (io_service_thread_.joinable())
            io_service_thread_.join();
    }
    
    // 禁止拷贝
    IoService(const IoService&) = delete;
    IoService& operator=(const IoService&) = delete;
    
    bool init() {

        if (initialized_) {
            return true;
        }

        std::lock_guard<std::mutex> lock(lock_);

        if (initialized_) {
            return true;
        }

        if(!io_service_ptr_)
            io_service_ptr_.reset(new boost::asio::io_service());

        // work_guard_ 保证io_service_run不会因为没有任务而退出
        work_guard_.reset(new boost::asio::io_service::work(*io_service_ptr_));

        // 创建io_service工作线程
        io_service_thread_ = std::thread(std::bind(&IoService::io_service_run, this));
        initialized_ = true;

        log_warning("IoService initialized ok.");
        return true;

    }

    boost::asio::io_service& io_service() {
        return *io_service_ptr_;
    }

    std::shared_ptr<boost::asio::io_service> io_service_ptr() {
        return io_service_ptr_;
    }

    void stop_io_service() {

        if(io_service_ptr_)
            io_service_ptr_->stop();
        work_guard_.reset();
    }


private:


    // 确保只初始化一次，double-lock-check
    std::mutex lock_;
    bool initialized_;

    // 启动一个io_service_，主要来处理定时器等常用服务
    std::thread io_service_thread_;
    std::shared_ptr<boost::asio::io_service> io_service_ptr_;

    // io_service如果没有任务，会直接退出执行，所以需要
    // 一个强制的work来持有之
    std::unique_ptr<boost::asio::io_service::work> work_guard_;

    void io_service_run() {

        log_warning("io_service thread running...");

        // io_service would not have had any work to do,
        // and consequently io_service::run() would have returned immediately.

        boost::system::error_code ec;
        io_service_ptr_->run(ec);

        log_warning("io_service thread terminated ...");
        log_warning("error_code: {%d} %s", ec.value(), ec.message().c_str());
    }

};

} // end namespace roo


#endif // __ROO_IO_SERVICE_H__
