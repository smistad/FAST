# Download and set up JQKTPlotter library

include(cmake/Externals.cmake)

ExternalProject_Add(jkqtplotter
        DEPENDS qt5
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/jkqtplotter
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/jkqtplotter
        GIT_REPOSITORY "https://github.com/jkriege2/JKQtPlotter.git"
        GIT_TAG "2b942e1e2d64c5fd45f5f70e53153b87299781ab"
        #UPDATE_COMMAND "" # Hack to avoid rebuild all the time on linux
        CMAKE_ARGS
          -DJKQtPlotter_BUILD_EXAMPLES:BOOL=OFF
          -DJKQtPlotter_BUILD_STATIC_LIBS=OFF
          -DJKQtPlotter_BUILD_SHARED_LIBS=ON
        CMAKE_CACHE_ARGS
          -DCMAKE_BUILD_TYPE:STRING=Release
          -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
          -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
          -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )

if(WIN32)
  list(APPEND LIBRARIES JKQTCommonSharedLib_Release.lib JKQTPlotterSharedLib_Release.lib JKQTFastPlotterSharedLib_Release.lib)
else()
  list(APPEND LIBRARIES libJKQTCommonSharedLib_Release.so libJKQTPlotterSharedLib_Release.so libJKQTFastPlotterSharedLib_Release.so)
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES jkqtplotter)
