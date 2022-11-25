# Download and set up TIFF library
include(cmake/ExternalLibJPEG.cmake)
include(cmake/Externals.cmake)

ExternalProject_Add(tiff
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tiff
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tiff
        DEPENDS libjpeg zlib
        GIT_REPOSITORY "https://gitlab.com/libtiff/libtiff/"
        GIT_TAG "v4.3.0"
        UPDATE_COMMAND "" # Hack to avoid rebuild all the time on linux
        CMAKE_ARGS
          -Djpeg=ON
          -Dold-jpeg=ON
          -Dzlib=ON
          -DHAVE_OPENGL=OFF
          -DJPEG_INCLUDE_DIR=${FAST_EXTERNAL_BUILD_DIR}/libjpeg/src/libjpeg/LibJPEG/9d/include/
          -DJPEG_LIBRARY_RELEASE=${FAST_EXTERNAL_BUILD_DIR}/libjpeg/src/libjpeg/LibJPEG/9d/lib/libjpeg.lib
          -DZLIB_INCLUDE_DIR=${FAST_EXTERNAL_INSTALL_DIR}/include/zlib/
          -DZLIB_LIBRARY_RELEASE=${FAST_EXTERNAL_INSTALL_DIR}/lib/zlib.lib
        CMAKE_CACHE_ARGS
          -DCMAKE_BUILD_TYPE:STRING=Release
          -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
          -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
          -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )

if(WIN32)
  list(APPEND LIBRARIES tiff.lib)
else()
  list(APPEND LIBRARIES ${CMAKE_SHARED_LIBRARY_PREFIX}tiff${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES tiff)
