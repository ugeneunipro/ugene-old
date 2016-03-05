cmake_minimum_required(VERSION 3.4)
project(U2Remote)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -nn) # do not display notes.
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt5 REQUIRED Core Gui Widgets Network)

add_definitions(-DBUILDING_U2REMOTE_DLL)

include_directories(src)
include_directories(../../include)
include_directories(../U2Private/src)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)

add_library(U2Remote SHARED ${HDRS} ${SRCS})

target_link_libraries(U2Remote
        U2Core U2Lang U2Gui U2Private
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)

