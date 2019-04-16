/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_SCAFFOLD_STATUS_H__
#define __ROO_SCAFFOLD_STATUS_H__

// 各个模块可以上报自己的状态，以json的格式输出来，然后这个模块进行数据汇总
// 内部模块采用 internal/status进行状态的展示

#include <mutex>
#include <memory>
#include <vector>
#include <functional>

#include <string>
#include <sstream>

#include <other/Log.h>


namespace roo {

// 配置动态更新回调函数接口类型
// 注意keyOut的构造，输出的时候按照这个排序

typedef std::function<int (std::string& module, std::string& name, std::string& val)> StatusCallable;


class Status : public std::enable_shared_from_this<Status> {

public:

    Status():
        lock_(),
        calls_() {
    }

    ~Status() { }

    // 禁止拷贝操作
    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;


    int attach_status_callback(const std::string& name, StatusCallable func);
    int collect_status(std::string& output);

    // self-used
    int module_status(std::string& module, std::string& name, std::string& val);

private:

    // 使用vector，这样能保留原始的注册顺序，甚至多次注册
    std::mutex lock_;
    std::vector<std::pair<std::string, StatusCallable>> calls_;
};



} // end namespace roo

#endif // __ROO_SCAFFOLD_STATUS_H__
