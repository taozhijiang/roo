#include <gmock/gmock.h>
#include <string>

#include <iostream>

#include <container/EQueue.h>

using namespace ::testing;
using namespace roo;

TEST(EQueueTest, EQueueSmokeTest) {
    
    EQueue<std::string> queue{};
    
    queue.PUSH("tao");
    queue.PUSH("kan");
    
    std::string val = queue.POP();
    
    ASSERT_THAT(val, Eq("tao"));
    ASSERT_THAT(queue.SIZE(), Eq(1));
}

