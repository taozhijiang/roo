/*-
 * Copyright (c) 2018-2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __CONSTRUCT_EXCEPTION_H__
#define __CONSTRUCT_EXCEPTION_H__

#include <exception>
#include <cstring>

// 这里不使用std::string，std::string只有移动操作符才保证不会抛出新的异常

namespace roo {
    
class ConstructException: public std::exception {
    
public:
    ConstructException(const char* msg) {
        ::memset(msg_, 0, kMaxExceptionMsgSize);
        std::strcpy(msg_, "ConstructException: ");
        if(msg)
            std::strncpy(msg_ + strlen("ConstructException: "), msg, 
                         kMaxExceptionMsgSize - strlen("ConstructException: ") - 1 );
    }
    
    ConstructException(const ConstructException& e) {
        ::memcpy(msg_, e.msg_, kMaxExceptionMsgSize);
    }
    
    virtual ~ConstructException() throw () /*noexcept*/ {
    }
    
    const char* what() const throw () /*noexcept*/ /*override*/ { 
        return msg_; 
    }
 
protected:
    static const size_t kMaxExceptionMsgSize = 1024;
    char msg_[kMaxExceptionMsgSize];
};


} // end namespace roo

#endif //__CONSTRUCT_EXCEPTION_H__
