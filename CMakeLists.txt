cmake_minimum_required (VERSION 2.8.11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x " )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Woverloaded-virtual -Wpointer-arith -Wwrite-strings -Werror=format -march=native " )


####################
### 修改编译选项 ###

# if want to see debug in verbose mode
# cmake -DCMAKE_VERBOSE_MAKEFILE=true ../
# if want to see build in detail
# make -- VERBOSE=1

option(BUILD_DEBUG "Build Project for Debug Propurse..." OFF)

if(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "build_type Debug flag: ${CMAKE_CXX_FLAGS_DEBUG}")
else(BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
    #add_definitions(-DNP_DEBUG)
    message(STATUS "build_type RelWithDebInfo flag: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif(BUILD_DEBUG)

include_directories( ../xtra_rhelz.x/include
                     ./ )

SET(SUBDIR connect concurrency container crypto string other)
foreach(dir ${SUBDIR})
    aux_source_directory(${dir} SUB_DIR_SOURCE)
    SET(ROO_SOURCE ${SUB_DIR_SOURCE})
endforeach()

message("full sources to be built: ")
foreach(item ${ROO_SOURCE})
    message(STATUS " > ${item}")
endforeach()


add_library (Roo STATIC ${ROO_SOURCE})

if(BUILD_DEBUG)
    message(STATUS "we will build examples and unit_tests ...")
    add_subdirectory( test )
endif(BUILD_DEBUG)

