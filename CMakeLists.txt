# /CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

project(
  qt-template
  VERSION 0.0.1
  LANGUAGES CXX C
)

string(TIMESTAMP YEAR "%Y")
string(TIMESTAMP MONTH "%m")
string(TIMESTAMP DAY "%d")
string(TIMESTAMP HOUR "%H")
string(TIMESTAMP MINUTE "%M")

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")
message(STATUS "TIME: ${YEAR}.${MONTH}.${DAY} ${HOUR}:${MINUTE}")
message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")
message(STATUS "CMAKE_PROJECT_NAME: ${CMAKE_PROJECT_NAME}")
message(STATUS "CMAKE_PROJECT_VERSION: ${CMAKE_PROJECT_VERSION}")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")

find_package(QT NAMES Qt6 QUIET COMPONENTS Core)
if(NOT Qt6_FOUND)
  find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
endif()

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")
message(STATUS "QT_VERSION: ${QT_VERSION}")
message(STATUS "QT_DIR: ${QT_DIR}")
message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")

enable_testing()

add_subdirectory(libs)
add_subdirectory(apps)
