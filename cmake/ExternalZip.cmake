# Download and set up zip library

include(cmake/Externals.cmake)

ExternalProject_Add(zip
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/zip
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/zip
        GIT_REPOSITORY "https://github.com/kuba--/zip.git"
        GIT_TAG "1d0fc2c043eb93f108282946e78466f63d9cf231"
        CMAKE_ARGS
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_DISABLE_TESTING=ON
        CMAKE_CACHE_ARGS
            -DDCMTK_MODULES:STRING=${MODULES}
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
)

list(APPEND FAST_EXTERNAL_DEPENDENCIES zip)
if(WIN32)
    list(APPEND LIBRARIES zip.lib)
else()
    list(APPEND LIBRARIES libzip.a)
endif()
