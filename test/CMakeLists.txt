cmake_minimum_required (VERSION 2.8.11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x " )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wpointer-arith -Wwrite-strings -Werror=format -march=native " )
include("${PROJECT_SOURCE_DIR}/help.cmake")
define_colors()

include_directories(
    ../
    ../../xtra_rhel7.x/self/include/
    ../../xtra_rhel7.x/self/include/curl-7.53.1
    ../../xtra_rhel7.x/boost_prefix/include/
    ../../xtra_rhel7.x/google_prefix/include/
)

link_directories(
    ../../xtra_rhel7.x/self/lib/
    ../../xtra_rhel7.x/self/lib/curl-7.53.1
    ../../xtra_rhel7.x/boost_prefix/lib/
    ../../xtra_rhel7.x/google_prefix/lib/
    ../lib
)

set (TEST_HOST_LIB Roo)

set (EXTRA_LIBS ${EXTRA_LIBS} boost_system)
set (EXTRA_LIBS ${EXTRA_LIBS} mysqlcppconn)
set (EXTRA_LIBS ${EXTRA_LIBS} glogb)
set (EXTRA_LIBS ${EXTRA_LIBS} gtest gmock gtest_main)
set (EXTRA_LIBS ${EXTRA_LIBS} dl curl pthread z crypto ssl)


add_individual_test(SqlConn)
add_individual_test(EQueue)
add_individual_test(HttpClient)
add_individual_test(Log)
add_individual_test(FilesystemUtil)
add_individual_test(InsaneBind)
add_individual_test(LruCache)
