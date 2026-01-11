# /libs/qtutils/CMakeLists.txt

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")

set(LIB_TARGET_NAME qtutils)
set(LIB_TARGET_VERSION 0.0.1)

message(STATUS "LIB_TARGET_NAME: ${LIB_TARGET_NAME}")
message(STATUS "LIB_TARGET_VERSION: ${LIB_TARGET_VERSION}")

find_package(
  Qt${QT_VERSION_MAJOR} REQUIRED
  COMPONENTS
  Core
  Gui
  Network
  Widgets
  Concurrent
)

# qt_standard_project_setup()
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(${LIB_TARGET_NAME} STATIC)

set_target_properties(
  ${LIB_TARGET_NAME} PROPERTIES
  VERSION ${LIB_TARGET_VERSION}
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(
  ${LIB_TARGET_NAME} PRIVATE
  $<$<CONFIG:Debug>:ENABLE_DEBUG_INFO>
  $<$<CONFIG:RelWithDebInfo>:ENABLE_DEBUG_INFO>
)

target_include_directories(
  ${LIB_TARGET_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(
  GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
# list(FILTER SRC_FILES EXCLUDE REGEX "")

target_sources(
  ${LIB_TARGET_NAME} PRIVATE
  ${SRC_FILES}
)

target_link_libraries(
  ${LIB_TARGET_NAME} PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Concurrent
)

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")
