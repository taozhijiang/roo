#include <gmock/gmock.h>
#include <string>

#include <other/InsaneBind.h>


using namespace ::testing;
using namespace roo;


TEST(InsaneBindTest, BindWithPortTest) {

    InsaneBind client {};

    uint16_t bind_port = client.port();
    std::cout << "return port " << bind_port << std::endl;
    ASSERT_THAT(bind_port, Gt(0));
}


TEST(InsaneBindTest, BindWithOutPortTest) {

    uint16_t expect_port = 40003;
    InsaneBind client (expect_port);

    uint16_t bind_port = client.port();
    std::cout << "return port " << bind_port << std::endl;
    ASSERT_THAT(bind_port, Eq(expect_port));

}


