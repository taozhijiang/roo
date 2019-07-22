/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_STRING_STR_UTIL_H__
#define __ROO_STRING_STR_UTIL_H__

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <other/Log.h>

// 类静态函数可以直接将函数定义丢在头文件中

namespace roo {

struct StrUtil {

    // 声明中带有format属性
    static std::string str_format(const char* fmt, ...) __attribute__((format(printf, 1, 2)));


    static size_t trim_whitespace(std::string& str) {

        size_t index = 0;
        size_t orig = str.size();

        // trim left whitespace
        for (index = 0; index < str.size() && isspace(str[index]); ++index)
        /* do nothing*/;
        str.erase(0, index);

        // trim right whitespace
        for (index = str.size(); index > 0 && isspace(str[index - 1]); --index)
        /* do nothing*/;
        str.erase(index);

        return orig - str.size();
    }


    template<typename T>
    static std::string to_string(const T& arg) {
        std::ostringstream ss;
        ss << arg;
        return ss.str();
    }


    static std::string pure_uri_path(std::string uri) {  // copy
        uri = boost::algorithm::trim_copy(boost::to_lower_copy(uri));
        while (uri[uri.size() - 1] == '/' && uri.size() > 1)  // 全部的小写字母，去除尾部
            uri = uri.substr(0, uri.size() - 1);

        return uri;
    }

    static std::string trim_lowcase(std::string str) {  // copy
        return boost::algorithm::trim_copy(boost::to_lower_copy(str));
    }

      // 删除host尾部的端口号
    static std::string drop_host_port(std::string host) {  // copy
        host = boost::algorithm::trim_copy(boost::to_lower_copy(host));
        auto pos = host.find(':');
        if (pos != std::string::npos) {
            host.erase(pos);
        }
        return host;
    }

};


// 长度安全版本，保证能够格式化成功，可能效率较慢
static inline std::string va_format(const char* fmt, ...) {

    std::string str;
    va_list ap;
    va_start(ap, fmt);

    size_t bufSize = 1024;
    while (true) {
        char buf[bufSize];
        // vsnprintf trashes the va_list, so copy it first
        va_list aq;
        va_copy(aq, ap);

        int r = vsnprintf(buf, bufSize, fmt, aq);
        va_end(aq);
        size_t r2 = size_t(r);
        if (r2 < bufSize) {
            str = std::string(buf);
            break;
        }

        bufSize = r2 + 1;
    }

    va_end(ap);
    return str;
}


} // roo

#endif // __ROO_STRING_STR_UTIL_H__
