cmake_minimum_required(VERSION 3.4)
project(breakpad)

include_directories(src)

if (WIN32)
    file(GLOB_RECURSE SRCS src/client/windows/*.cc)
    file(GLOB_RECURSE HDRS src/client/windows/*.h)
elseif (APPLE)
    file(GLOB_RECURSE SRCS src/client/mac/*.cc)
    file(GLOB_RECURSE HDRS src/client/mac/*.h)
elseif (UNIX)
    file(GLOB_RECURSE SRCS src/client/linux/*.cc)
    file(GLOB_RECURSE HDRS src/client/linux/*.h)
endif ()

add_library(breakpad SHARED ${HDRS} ${SRCS})

target_link_libraries(breakpad)
