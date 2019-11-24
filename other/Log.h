/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_OTHER_LOG_H__
#define __ROO_OTHER_LOG_H__

#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <cstring>
#include <cstddef>

#include <string>
#include <algorithm>

#include <boost/current_function.hpp>

#include <syslog.h>
#include <glogb/logging.h>

// LOG_EMERG   0   system is unusable
// LOG_ALERT   1   action must be taken immediately
// LOG_CRIT    2   critical conditionitions
// LOG_ERR     3   error conditionitions
// LOG_WARNING 4   warning conditionitions
// LOG_NOTICE  5   normal, but significant, conditionition
// LOG_INFO    6   informational message
// LOG_DEBUG   7   debug-level message


// 这里主要是用于兼容老的日志接口，新服务直接使用GLog接口

namespace roo {

bool log_init(int log_level, std::string module = "",
              std::string log_dir = "./log", int facility = -1);

void log_close();

void log_api(int priority, const char* file, int line, const char* func, const char* msg, ...)
__attribute__((format(printf, 5, 6)));

void log_api_if(int priority, bool condition, const char* file, int line, const char* func, const char* msg, ...)
__attribute__((format(printf, 6, 7)));


#define log_emerg(...)   /*avoid*/ log_api( LOG_ALERT, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_alert(...)             log_api( LOG_ALERT, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_crit(...)    /*avoid*/ log_api( LOG_ALERT, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_err(...)               log_api( LOG_ERR  , __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_warning(...)           log_api( LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_notice(...)  /*avoid*/ log_api( LOG_WARNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_info(...)              log_api( LOG_INFO , __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_debug(...)   /*avoid*/ log_api( LOG_INFO , __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define log_emerg_if(condition, ...)   /*avoid*/ \
    log_api_if( LOG_ALERT, !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_alert_if(condition, ...)             \
    log_api_if( LOG_ALERT, !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_crit_if(condition, ...)    /*avoid*/ \
    log_api_if( LOG_ALERT, !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_err_if(condition, ...)               \
    log_api_if( LOG_ERR  , !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_warning_if(condition, ...)           \
    log_api_if( LOG_WARNING, !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_notice_if(condition, ...)  /*avoid*/ \
    log_api_if( LOG_WARNING, !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_info_if(condition, ...)              \
    log_api_if( LOG_INFO , !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define log_debug_if(condition, ...)   /*avoid*/ \
    log_api_if( LOG_INFO , !!(condition), __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)


} // roo


// Log to ALERT level and abort the process.
#define PANIC(...) do { \
    roo::log_api( LOG_ALERT, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); \
    roo::log_api( LOG_ALERT, __FILE__, __LINE__, __FUNCTION__, "Service Exiting..."); \
    ::abort(); \
} while (0)

#endif // __ROO_OTHER_LOG_H__
