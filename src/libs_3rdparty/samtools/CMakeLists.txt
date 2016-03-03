cmake_minimum_required(VERSION 3.4)
project(samtools)

include_directories(src)
include_directories(src/samtools)
include_directories(../../include)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)

add_library(samtools STATIC ${HDRS} ${SRCS})

target_link_libraries(samtools
        zlib)
