# CMake build is not officially supported by UGENE and
# can only be used for developement (for example to enable CLion IDE integration)
#
# For CLion developers: add Qt5_DIR variable to File->Settings->Build,Execution,Deployment->CMake pointing to valid QT dir.

cmake_minimum_required(VERSION 3.4)
project(ugene)

add_definitions(
        -DUGENE_VERSION=1.22.0-dev
        -DUGENE_MIN_VERSION_SQLITE=1.13.0
        -DUGENE_MIN_VERSION_MYSQL=1.16.0
        -DUGENE_VER_MAJOR=1
        -DUGENE_VER_MINOR=22
        -DUGENE_VER_PATCH=0
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_subdirectory(src/libs_3rdparty/sqlite3)
add_subdirectory(src/libs_3rdparty/zlib)
add_subdirectory(src/corelibs/U2Core)
