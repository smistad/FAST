# Download and set up libfreenect2

include(cmake/Externals.cmake)

ExternalProject_Add(freenect2
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/freenect2
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/freenect2
        GIT_REPOSITORY "https://github.com/OpenKinect/libfreenect2.git"
        GIT_TAG "v0.2.0"
        CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
        -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
        -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )

#list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/eigen3/)
list(APPEND FAST_EXTERNAL_DEPENDENCIES freenect2)
