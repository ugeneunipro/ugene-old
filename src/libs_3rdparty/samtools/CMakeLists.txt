cmake_minimum_required(VERSION 3.4)
project(samtools)

include_directories(src)
include_directories(src/samtools)
include_directories(../../include)

add_definitions(
        -D_FILE_OFFSET_BITS=64
        -D_LARGEFILE64_SOURCE
        -D_USE_KNETFILE
)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)

add_library(samtools STATIC ${HDRS} ${SRCS})

target_link_libraries(samtools
        zlib)
