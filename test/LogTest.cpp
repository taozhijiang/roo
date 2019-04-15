#include <gmock/gmock.h>
#include <string>

#include <other/Log.h>


using namespace ::testing;
using namespace roo;


TEST(LogTest, RawTest) {

    log_init(LOG_DEBUG, "", "./log", LOG_LOCAL6);

    log_debug("log_debug msg");
    log_info("log_info msg");
    log_err("log_error msg");

    log_info_if( 1 < 0, "you should no see this info msg");
    log_info_if( 1 > 0, "good info msg when condition met");
}


