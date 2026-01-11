# /apps/app/CMakeLists.txt

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")

set(APP_TARGET_NAME app)
set(APP_TARGET_VERSION 0.0.1)

message(STATUS "APP_TARGET_NAME: ${APP_TARGET_NAME}")
message(STATUS "APP_TARGET_VERSION: ${APP_TARGET_VERSION}")

find_package(
  Qt${QT_VERSION_MAJOR} REQUIRED
  COMPONENTS
  Core
  Gui
  Network
  Widgets
)

# qt_standard_project_setup()
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${APP_TARGET_NAME})

set_target_properties(
  ${APP_TARGET_NAME} PROPERTIES
  VERSION ${APP_TARGET_VERSION}
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  WIN32_EXECUTABLE TRUE
)

target_include_directories(
  ${APP_TARGET_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

file(
  GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui
)
# list(FILTER SRC_FILES EXCLUDE REGEX "")

target_sources(
  ${APP_TARGET_NAME} PRIVATE
  ${SRC_FILES}
)

# qt_add_resources(${PROJECT_NAME} PREFIX "/" FILES "")
target_sources(
  ${APP_TARGET_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/res/images/images.qrc
)

target_link_libraries(
  ${APP_TARGET_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Widgets
  qtutils
)

include(GNUInstallDirs)
include(InstallRequiredSystemLibraries)

if(UNIX)
  set_target_properties(${APP_TARGET_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    BUILD_WITH_INSTALL_RPATH FALSE
    SKIP_BUILD_RPATH FALSE
    BUILD_RPATH_USE_ORIGIN TRUE
  )
endif()

install(
  TARGETS ${APP_TARGET_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# install(
#   DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/fonts/
#   DESTINATION ${CMAKE_INSTALL_BINDIR}/fonts
# )

if(QT_VERSION_MAJOR GREATER 6)
  qt_generate_deploy_app_script(
    TARGET ${APP_TARGET_NAME}
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
  )
  install(SCRIPT ${deploy_script})
endif()

# if(USE_QT5)
#   include(cmake/linux-deploy-qt5.cmake)
# elseif(USE_QT6)
#   include(cmake/linux-deploy-qt6.cmake)
# else()
#   include(cmake/linux-deploy-qt6.cmake)
# endif()

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")
