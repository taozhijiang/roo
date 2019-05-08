/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#include <sstream>
#include <iostream>

#include <scaffold/Setting.h>
#include <scaffold/Status.h>

namespace roo {


bool Setting::init(std::string file) {

    cfg_file_ = file;

    setting_ptr_.reset( new libconfig::Config() );
    if (!setting_ptr_) {
        log_err("create libconfig failed.");
        return false;
    }

    // try load and explain the cfg_file first.
    try {
        setting_ptr_->readFile(file.c_str());
    } catch(libconfig::FileIOException &fioex) {
        fprintf(stderr, "I/O error while reading file: %s.", file.c_str());
        log_err( "I/O error while reading file: %s.", file.c_str());
        setting_ptr_.reset();
    } catch(libconfig::ParseException &pex) {
        fprintf(stderr, "Parse error at %d - %s", pex.getLine(), pex.getError());
        log_err( "Parse error at %d - %s", pex.getLine(), pex.getError());
        setting_ptr_.reset();
    }

    // when init, parse conf failed was critical.
    if (!setting_ptr_) {
        return false;
    }

    return true;
}

int Setting::update_runtime_setting() {

    if (cfg_file_.empty()) {
        log_err("param cfg_file is not set, may not initialized ???");
        return -1;
    }

    std::lock_guard<std::mutex> lock(lock_);

    if (in_process_) {
        log_err("!!! already in process, please try again later!");
        return 0;
    }

    auto setting = load_cfg_file();
    if (!setting) {
        in_process_ = false;
        log_err("load config file %s failed.", cfg_file_.c_str());
        return false;
    }

    // 重新读取配置并且解析成功之后，才更新这个指针
    std::swap(setting, setting_ptr_);
    last_update_time_ = ::time(NULL);

    int ret = 0;
    for (auto it = calls_.begin(); it != calls_.end(); ++it) {
        ret += (it->second)(*setting_ptr_); // call it!
    }

    log_warning("Setting::update_runtime_conf total callback return: %d", ret);
    in_process_ = false;

    return ret;
}

int Setting::attach_runtime_callback(const std::string& name, SettingUpdateCallable func) {

    if (name.empty() || !func){
        log_err("invalid name or func param.");
        return -1;
    }

    std::lock_guard<std::mutex> lock(lock_);
    calls_.push_back({name, func});
    log_info("register runtime for %s success.",  name.c_str());

    return 0;
}


int Setting::module_status(std::string& module, std::string& name, std::string& val) {

    module = "roo";
    name   = "Setting";

    std::stringstream ss;
    ss << "attached runtime update: " << std::endl;

    int i = 1;
    for (auto it = calls_.begin(); it != calls_.end(); ++it) {
        ss << "\t" << i++ << ". "<< it->first << std::endl;
    }

    val = ss.str();
    return 0;
}


std::shared_ptr<libconfig::Config> Setting::load_cfg_file() {

    std::shared_ptr<libconfig::Config> setting = std::make_shared<libconfig::Config>();
    if (!setting) {
        log_err("create libconfig::Config instance failed!");
        return setting; // nullptr
    }

    try {
        setting->readFile(cfg_file_.c_str());
    } catch (libconfig::FileIOException& fioex) {
        log_err("I/O error while reading file: %s.", cfg_file_.c_str());
        setting.reset();
    } catch (libconfig::ParseException& pex) {
        log_err("Parse error at %d - %s", pex.getLine(), pex.getError());
        setting.reset();
    }

    return setting;
}


} // end namespace roo
