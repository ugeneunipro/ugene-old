cmake_minimum_required(VERSION 3.4)
project(ugenedb)

include_directories(src)

file(GLOB_RECURSE SRCS src/*.c)
file(GLOB_RECURSE HDRS src/*.h)

add_library(ugenedb STATIC ${HDRS} ${SRCS})

target_link_libraries(ugenedb)

