# CMake build is not officially supported by UGENE and
# can only be used for developement (for example to enable CLion IDE integration)
#
# For CLion developers: add Qt5_DIR variable to File->Settings->Build,Execution,Deployment->CMake pointing to valid QT dir.

cmake_minimum_required(VERSION 3.4)
project(ugene)

set(UGENE_VERSION 1.22.0-dev)
set(UGENE_VER_MAJOR 1)
set(UGENE_VER_MINOR 22)
set(UGENE_VER_PATCH 0)

set(UGENE_MIN_VERSION_SQLITE 1.13.0)
set(UGENE_MIN_VERSION_MYSQL 1.16.0)

add_definitions(
        -DUGENE_VERSION=${UGENE_VERSION}
        -DUGENE_VER_MAJOR=${UGENE_VER_MAJOR}
        -DUGENE_VER_MINOR=${UGENE_VER_MINOR}
        -DUGENE_VER_PATCH=${UGENE_VER_PATCH}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_subdirectory(src/libs_3rdparty/sqlite3)
add_subdirectory(src/libs_3rdparty/zlib)
add_subdirectory(src/corelibs/U2Core)
