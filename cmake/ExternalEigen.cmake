# Download and set up Eigen

include(cmake/Externals.cmake)

ExternalProject_Add(eigen
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/eigen
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/eigen
        #GIT_REPOSITORY "https://github.com/RLovelett/eigen.git"
        #GIT_TAG "a46d2e7337c4656f00abe54a8115f6d76153a048"
        URL "https://bitbucket.org/eigen/eigen/get/f3a22f35b044.tar.gz"
        INSTALL_DIR ${FAST_EXTERNAL_INSTALL_DIR}
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
            -DBUILD_TESTING:BOOL=OFF
)

list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/eigen3/)
list(APPEND FAST_EXTERNAL_DEPENDENCIES eigen)