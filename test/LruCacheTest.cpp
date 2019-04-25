#include <gmock/gmock.h>
#include <string>

#include <container/LruCache.h>


using namespace ::testing;
using namespace roo;


TEST(LruCacheTest, LRUSmokeTest) {

    LruCache<std::string, std::string> caches(10, 1000);

    ASSERT_THAT(caches.insert("key1", "value1"), Eq(true));
    ASSERT_THAT(caches.insert("key2", "value2"), Eq(true));
    ASSERT_THAT(caches.insert("key1", "value3"), Eq(false));

    std::string value;
    ASSERT_THAT(caches.find("key2", value) && value == "value2", Eq(true));

    ASSERT_THAT(caches.item_count(), Eq(2));

    ASSERT_THAT(caches.find("key1"), Eq(true));
    ASSERT_THAT(caches.find("key3"), Eq(false));

    ASSERT_THAT(caches.insert_or_update("key1", "value3"), Eq(true));
    ASSERT_THAT(caches.find("key1", value) && value == "value3", Eq(true));
}


