#include <gmock/gmock.h>
#include <string>

#include <container/LruCache.h>
#include <container/LruCacheMem.h>

using namespace ::testing;
using namespace roo;


TEST(LruCacheTest, LRUSmokeTest) {

    LruCache<std::string, std::string> caches(10);

    ASSERT_THAT(caches.insert("key1", "value1"), Eq(true));
    ASSERT_THAT(caches.insert("key2", "value2"), Eq(true));
    ASSERT_THAT(caches.insert("key1", "value3"), Eq(false));

    std::string value;
    ASSERT_THAT(caches.find("key2", value) && value == "value2", Eq(true));

    ASSERT_THAT(caches.total_count(), Eq(2));

    ASSERT_THAT(caches.find("key1"), Eq(true));
    ASSERT_THAT(caches.find("key3"), Eq(false));

    ASSERT_THAT(caches.insert_or_update("key1", "value3"), Eq(true));
    ASSERT_THAT(caches.find("key1", value) && value == "value3", Eq(true));
}



TEST(LruCacheTest, LruCacheMemTest) {

    LruCacheMem<std::string, std::string> caches(10);

    ASSERT_THAT(caches.total_mem_used(), Eq(0));

    ASSERT_THAT(caches.insert("key1", "value1"), Eq(true));
    ASSERT_THAT(caches.insert("key2", "value2"), Eq(true));
    ASSERT_THAT(caches.total_mem_used(), Gt(0));

    size_t mem = caches.total_mem_used();
    ASSERT_THAT(caches.insert_or_update("key2", "value4"), Eq(true));

    std::string value;
    ASSERT_THAT(caches.find("key2", value) && value == "value4", Eq(true));
    ASSERT_THAT(caches.total_mem_used(), Eq(mem));
    std::cout << mem << std::endl;

    ASSERT_THAT(caches.insert_or_update("key2", "value333"), Eq(true));
    ASSERT_THAT(caches.total_mem_used(), Eq(mem + 2));

}
