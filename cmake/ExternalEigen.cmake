# Download and set up Eigen

include(cmake/Externals.cmake)

ExternalProject_Add(eigen
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/eigen
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/eigen
        GIT_REPOSITORY "https://github.com/RLovelett/eigen.git"
        GIT_TAG "3.3-rc2"
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
)

list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/eigen3/)
list(APPEND FAST_EXTERNAL_DEPENDENCIES eigen)