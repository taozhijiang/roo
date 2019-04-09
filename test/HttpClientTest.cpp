#include <gmock/gmock.h>
#include <string>

#include <iostream>

#include <other/HttpClient.h>

using namespace ::testing;
using namespace roo;

TEST(HttpClientTest, HttpClientSmokeTest) {

    const char* url_404 = "http://www.example.com/bb";
    const char* url_200 = "http://www.example.com/index.html";

    auto client = std::make_shared<HttpClient>();
    int code = client->GetByHttp(url_404);

    ASSERT_THAT(code, Ne(0));
    
    code = client->GetByHttp(url_200);
    ASSERT_THAT(code, Eq(0));
}

