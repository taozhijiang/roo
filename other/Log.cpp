/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#include <xtra_rhel.h>

#include <vector>
#include <boost/algorithm/string.hpp>

#include <other/Log.h>

#include <glog_syslog/logging.h>


namespace roo {


// The use of openlog() is optional; it will automatically be called by syslog() if necessary.
bool log_init(int log_level, std::string module,
              std::string log_dir, int facility) {

    if (log_level >= LOG_INFO) {
        FLAGS_minloglevel = 0; // INFO
    } else if (log_level >= LOG_WARNING) {
        FLAGS_minloglevel = 1; // WARNING
    } else if (log_level >= LOG_ERR) {
        FLAGS_minloglevel = 2; // ERROR
    } else if (log_level >= LOG_CRIT) {
        FLAGS_minloglevel = 3; // FATAL
    } else {
        FLAGS_minloglevel = 0;
    }

    if (!log_dir.empty())
        FLAGS_log_dir = log_dir;

    FLAGS_syslog_facility = facility;

    if (!module.empty()) {
        google::InitGoogleLogging(module.c_str());
    } else {
        google::InitGoogleLogging("./log");
    }

    return true;
}

void log_close() {
}

static const std::size_t MAX_LOG_BUF_SIZE = (16 * 1024 - 2);

void log_api(int priority, const char* file, int line, const char* func, const char* msg, ...) {

    char buf[MAX_LOG_BUF_SIZE + 2] = { 0, };

    va_list arg_ptr;
    va_start(arg_ptr, msg);
    vsnprintf(buf, MAX_LOG_BUF_SIZE, msg, arg_ptr);
    va_end(arg_ptr);

#if 0
    if (priority <= LOG_CRIT)
        LEGACY_FATAL(file, line, func).stream() << buf;
    else if (priority <= LOG_ERR)
        LEGACY_ERROR(file, line, func).stream() << buf;
    else if (priority <= LOG_NOTICE)
        LEGACY_WARNING(file, line, func).stream() << buf;
    else if (priority <= LOG_DEBUG)
        LEGACY_INFO(file, line, func).stream() << buf;
    else
    /* ?? */;
#endif

    if (priority <= LOG_CRIT)
        LEGACY(FATAL, file, line, func) << buf;
    else if (priority <= LOG_ERR)
        LEGACY(ERROR, file, line, func) << buf;
    else if (priority <= LOG_NOTICE)
        LEGACY(WARNING, file, line, func) << buf;
    else if (priority <= LOG_DEBUG)
        LEGACY(INFO, file, line, func) << buf;
    else
    /* ?? */;
}

void log_api_if(int priority, bool condition, const char* file, int line, const char* func, const char* msg, ...) {

    char buf[MAX_LOG_BUF_SIZE + 2] = { 0, };

    va_list arg_ptr;
    va_start(arg_ptr, msg);
    vsnprintf(buf, MAX_LOG_BUF_SIZE, msg, arg_ptr);
    va_end(arg_ptr);

#if 0
    if (priority <= LOG_CRIT)
        LEGACY_FATAL_IF(condition, file, line, func).stream() << buf;
    else if (priority <= LOG_ERR)
        LEGACY_ERROR_IF(condition, file, line, func).stream() << buf;
    else if (priority <= LOG_NOTICE)
        LEGACY_WARNING_IF(condition, file, line, func).stream() << buf;
    else if (priority <= LOG_DEBUG)
        LEGACY_INFO_IF(condition, file, line, func).stream() << buf;
    else
    /* ?? */;
#endif

    if (priority <= LOG_CRIT)
        LEGACY_IF(FATAL, condition, file, line, func) << buf;
    else if (priority <= LOG_ERR)
        LEGACY_IF(ERROR, condition, file, line, func) << buf;
    else if (priority <= LOG_NOTICE)
        LEGACY_IF(WARNING, condition, file, line, func) << buf;
    else if (priority <= LOG_DEBUG)
        LEGACY_IF(INFO, condition, file, line, func) << buf;
    else
    /* ?? */;
}


} // roo

