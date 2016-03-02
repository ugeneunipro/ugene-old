cmake_minimum_required(VERSION 3.4)
project(U2Lang)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -nn) # do not display notes.

find_package(Qt5 REQUIRED Core Xml Script)

add_definitions(-DBUILDING_U2LANG_DLL)

include_directories(src)
include_directories(../../include)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)
qt5_add_resources(RCC_SRCS src/U2Lang.qrc)

add_library(U2Lang SHARED ${HDRS} ${SRCS})

target_link_libraries(U2Lang
        U2Core
        Qt5::Core Qt5::Xml Qt5::Script)

