cmake_minimum_required (VERSION 2.8.11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x " )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Woverloaded-virtual -Wpointer-arith -Wwrite-strings -Werror=format -march=native " )

add_definitions(-DNP_DEBUG)
include_directories( ../xtra_rhelz.x/include
                     ./ )

SET(SUBDIR connect concurrency container crypto string other)
foreach(dir ${SUBDIR})
    aux_source_directory(${dir} SUB_DIR_SOURCE)
    SET(ROO_SOURCE ${SUB_DIR_SOURCE})
endforeach()

message("full sources to be built: ${ROO_SOURCE}")
add_library (Roo STATIC ${ROO_SOURCE})

add_subdirectory( test )
