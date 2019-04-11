cmake_minimum_required (VERSION 2.8.11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x " )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wpointer-arith -Wwrite-strings -Werror=format -march=native " )
include("${PROJECT_SOURCE_DIR}/help.cmake")
define_colors()

include_directories(
    ../
    ../../xtra_rhelz.x/include/
    ../../xtra_rhelz.x/include/google/
)

link_directories(
    ../../xtra_rhelz.x/libs/
    ../../xtra_rhelz.x/libs/boost/
    ../../xtra_rhelz.x/libs/google/
    ../lib
)

set (TEST_HOST_LIB Roo)

set (EXTRA_LIBS ${EXTRA_LIBS} curl)
set (EXTRA_LIBS ${EXTRA_LIBS} boost_system)
set (EXTRA_LIBS ${EXTRA_LIBS} mysqlcppconn)
set (EXTRA_LIBS ${EXTRA_LIBS} gtest gmock gtest_main)


add_individual_test(SqlConn)
add_individual_test(EQueue)
add_individual_test(HttpClient)