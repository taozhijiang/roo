/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_SCAFFOLD_SETTING_H__
#define __ROO_SCAFFOLD_SETTING_H__

#include <memory>
#include <mutex>
#include <vector>
#include <functional>

#include <libconfig.h++>

#include <other/Log.h>


// 值拷贝

namespace roo {

// 配置动态更新回调函数接口类型
typedef std::function<int(const libconfig::Config& setting)> SettingUpdateCallable;

class Setting : public std::enable_shared_from_this<Setting> {

public:

    Setting() :
        cfg_file_(),
        setting_ptr_(),
        last_update_time_(0),
        in_process_(false),
        lock_(),
        calls_() {
    }

    ~Setting() {
    }

    // 禁止拷贝操作
    Setting(const Setting&) = delete;
    Setting& operator=(const Setting&) = delete;


    // provide libconfig formate
    bool init(std::string file);

    // 配置更新的调用入口函数
    int update_runtime_setting();
    int attach_runtime_callback(const std::string& name, SettingUpdateCallable func);


    // 提供本模块所有注册的回调任务的信息
    int module_status(std::string& module, std::string& name, std::string& val);


    std::shared_ptr<libconfig::Config> get_setting() {

        // try update new conf first
        // 只有更新成功了，conf_ptr_才会指向新的配置信息
        load_cfg_file();

        std::lock_guard<std::mutex> lock(lock_);
        return setting_ptr_;
    }

    // 模板函数，方便快速简洁获取配置
    // 这边保证conf_ptr_始终是可用的，否则整个系统在初始化的时候就失败了
    template<typename T>
    bool get_setting_value(const std::string& key, T& t) {

        // 超过10min，重新读取配置文件，尝试
        if (last_update_time_ < ::time(NULL) - 10 * 60) {

            log_info("reloading cfg file %s, last update interval was %ld secs",
                      cfg_file_.c_str(),
                      ::time(NULL) - last_update_time_);

            auto setting = load_cfg_file();
            if (!setting) {
                log_err("load config file %s failed.", cfg_file_.c_str());
                log_err("we try best to return old staged value.");
            } else {
                std::lock_guard<std::mutex> lock(lock_);
                std::swap(setting, setting_ptr_);
                last_update_time_ = ::time(NULL);
            }
        }

        // anyway, return sth
        std::lock_guard<std::mutex> lock(lock_);
        if (setting_ptr_->lookupValue(key, t)) {
            return true;
        }

        t = T {};
        return false;
    }


private:
    std::shared_ptr<libconfig::Config> load_cfg_file();

private:
    std::string cfg_file_;
    std::shared_ptr<libconfig::Config> setting_ptr_;
    time_t last_update_time_;

    bool in_process_;
    std::mutex lock_;
    std::vector<std::pair<std::string, SettingUpdateCallable>> calls_;
};

} // end namespace roo

#endif // __ROO_SCAFFOLD_SETTING_H__
