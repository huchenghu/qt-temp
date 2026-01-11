# linux-deploy-qt6.cmake

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")
message(STATUS "linux deploy qt6")
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  include(GNUInstallDirs)

  # 1. 安装qt.conf
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/qt.conf
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

  # 2. 安装lib, plugins, libexec
  get_target_property(QT_CORE_LOCATION Qt${QT_VERSION_MAJOR}::Core LOCATION)
  get_filename_component(QT_LIB_DIR "${QT_CORE_LOCATION}" DIRECTORY)
  set(QT_PLUGINS_DIR "${QT_LIB_DIR}/qt6/plugins")
  set(QT_LIBEXEC_DIR "/usr/lib/qt6/libexec")

  file(GLOB QT_LIBS "${QT_LIB_DIR}/libQt*.so*")
  install(
    FILES ${QT_LIBS}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  if(EXISTS "${QT_PLUGINS_DIR}/")
    install(
      DIRECTORY "${QT_PLUGINS_DIR}/"
      DESTINATION "${CMAKE_INSTALL_PREFIX}/plugins"
    )
  endif()

  if(EXISTS "${QT_LIBEXEC_DIR}/")
    install(
      DIRECTORY "${QT_LIBEXEC_DIR}/"
      DESTINATION "${CMAKE_INSTALL_PREFIX}/libexec"
      USE_SOURCE_PERMISSIONS
    )
  endif()

  # 3. 安装QtWebEngine resources, translations
  get_target_property(QT_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
  execute_process(
    COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_DATA
    OUTPUT_VARIABLE QT_INSTALL_DATA
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  install(
    DIRECTORY "${QT_INSTALL_DATA}/resources/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/resources
  )

  install(
    DIRECTORY "${QT_INSTALL_DATA}/translations/"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/translations
  )

  # 4. 通过ldd收集并安装所有依赖库

  message(STATUS "QT_CORE_LOCATION: ${QT_CORE_LOCATION}")
  message(STATUS "QT_LIB_DIR: ${QT_LIB_DIR}")
  message(STATUS "QT_PLUGINS_DIR: ${QT_PLUGINS_DIR}")
  message(STATUS "QT_LIBEXEC_DIR: ${QT_LIBEXEC_DIR}")
  message(STATUS "QT_INSTALL_DATA: ${QT_INSTALL_DATA}")
endif()

message(STATUS "-- -- -- -- -- -- -- -- -- -- -- -- -- -- --")
