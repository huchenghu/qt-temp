# /CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

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

option(USE_QT5 "use Qt5" OFF)
option(USE_QT6 "use Qt6" OFF)
if(USE_QT5)
  find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
elseif(USE_QT6)
  find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
else()
  find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
endif()

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")
message(STATUS "QT_VERSION: ${QT_VERSION}")
message(STATUS "QT_DIR: ${QT_DIR}")
message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")

add_subdirectory(libs)
add_subdirectory(apps)
