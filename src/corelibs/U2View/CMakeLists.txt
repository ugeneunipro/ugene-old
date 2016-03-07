cmake_minimum_required(VERSION 3.4)
project(U2View)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -nn) # do not display notes.
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt5 REQUIRED Core Gui Widgets Xml Svg WebKit WebKitWidgets PrintSupport)

add_definitions(-DBUILDING_U2DESIGNER_DLL)

include_directories(src)
include_directories(../../include)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)

qt5_add_resources(RCC_SRCS src/U2Gui.qrc)

add_library(U2View SHARED ${HDRS} ${SRCS})

target_link_libraries(U2View
        U2Core U2Algorithm U2Formats U2Remote U2Lang U2Gui
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml Qt5::Svg Qt5::WebKit Qt5::WebKitWidgets Qt5::PrintSupport)

