# Download and set up JQKTPlotter library

include(cmake/Externals.cmake)

if(FAST_BUILD_ALL_DEPENDENCIES)
ExternalProject_Add(jkqtplotter
        DEPENDS qt5
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/jkqtplotter
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/jkqtplotter
        GIT_REPOSITORY "https://github.com/jkriege2/JKQtPlotter.git"
        GIT_TAG "fc7622e901cec7ed68abe6b2d95ea20ce30490ed"
        #UPDATE_COMMAND "" # Hack to avoid rebuild all the time on linux
        CMAKE_ARGS
          -DJKQtPlotter_BUILD_EXAMPLES:BOOL=OFF
          -DJKQtPlotter_BUILD_STATIC_LIBS=OFF
          -DJKQtPlotter_BUILD_SHARED_LIBS=ON
        CMAKE_CACHE_ARGS
          -DQt5_DIR=${Qt5_DIR}
          -DCMAKE_BUILD_TYPE:STRING=Release
          -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
          -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
          -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
  )
else()
if(WIN32)
else()
  ExternalProject_Add(jkqtplotter
          PREFIX ${FAST_EXTERNAL_BUILD_DIR}/jkqtplotter
          URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL}/linux/jkqtplotter_2020.10_glibc2.27.tar.xz
          URL_HASH SHA256=08618b4a56b20c98b352c3edd2e80d899d270bbfa86648326730b0881a3a3f6e
          UPDATE_COMMAND ""
          CONFIGURE_COMMAND ""
          BUILD_COMMAND ""
          # On install: Copy contents of each subfolder to the build folder
          INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${FAST_EXTERNAL_INSTALL_DIR}/include COMMAND
            ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib COMMAND
            ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licences ${FAST_EXTERNAL_INSTALL_DIR}/licences
  )
endif()
endif()

if(WIN32)
  list(APPEND LIBRARIES JKQTCommonSharedLib_Release.lib JKQTPlotterSharedLib_Release.lib JKQTFastPlotterSharedLib_Release.lib)
else()
  list(APPEND LIBRARIES libJKQTCommonSharedLib_Release.so libJKQTPlotterSharedLib_Release.so libJKQTFastPlotterSharedLib_Release.so)
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES jkqtplotter)
