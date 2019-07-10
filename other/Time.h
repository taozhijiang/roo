/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_OTHER_TIME_H__
#define __ROO_OTHER_TIME_H__

#include <ctime>

#include <xtra_rhel.h>

namespace roo {

static inline std::string unixtime_str(time_t time) {
    char mbstr[32] {};
    ::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&time));
    return std::string(mbstr);
}

} // roo

#endif // __ROO_OTHER_TIME_H__
