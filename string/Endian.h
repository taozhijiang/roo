/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_STRING_ENDIAN_H__
#define __ROO_STRING_ENDIAN_H__


#include <string>

// 类静态函数可以直接将函数定义丢在头文件中

namespace roo {

struct Endian {

    static inline std::string uint64_to_net(uint64_t num) {
        char buf[8] {};
        uint64_t n_num = htobe64(num);
        ::memcpy(buf, reinterpret_cast<char*>(&n_num), sizeof(uint64_t));
        return std::string(buf, 8);
    }

    static uint64_t uint64_from_net(const std::string& str) {
        uint64_t num;
        ::memcpy(reinterpret_cast<char*>(&num), str.c_str(), sizeof(uint64_t));
        return be64toh(num);
    }

    static inline std::string uint32_to_net(uint32_t num) {
        char buf[4] {};
        uint32_t n_num = htobe32(num);
        ::memcpy(buf, reinterpret_cast<char*>(&n_num), sizeof(uint32_t));
        return std::string(buf, 8);
    }

    static uint32_t uint32_from_net(const std::string& str) {
        uint32_t num;
        ::memcpy(reinterpret_cast<char*>(&num), str.c_str(), sizeof(uint32_t));
        return be32toh(num);
    }

    static inline std::string uint16_to_net(uint16_t num) {
        char buf[2] {};
        uint16_t n_num = htobe16(num);
        ::memcpy(buf, reinterpret_cast<char*>(&n_num), sizeof(uint16_t));
        return std::string(buf, 8);
    }

    static uint16_t uint16_from_net(const std::string& str) {
        uint16_t num;
        ::memcpy(reinterpret_cast<char*>(&num), str.c_str(), sizeof(uint16_t));
        return be16toh(num);
    }

};


} // roo

#endif // __ROO_STRING_ENDIAN_H__
