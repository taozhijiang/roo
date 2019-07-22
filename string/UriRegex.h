/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_STRING_URI_REGEX_H__
#define __ROO_STRING_URI_REGEX_H__

#include <libconfig/libconfig.h++>


#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// 类静态函数可以直接将函数定义丢在头文件中

namespace roo {

class UriRegex : public boost::regex {
public:
    explicit UriRegex(const std::string& regexStr) :
        boost::regex(regexStr), str_(regexStr) {
    }

    std::string str() const {
        return str_;
    }

private:
    std::string str_;
};


} // roo

#endif // __ROO_STRING_URI_REGEX_H__
