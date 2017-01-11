# Download and set up GLEW

include(cmake/Externals.cmake)

ExternalProject_Add(glew
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/glew
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/glew
        # The original GLEW library of https://github.com/nigels-com/glew/ has not proper cmake setup
        # Use this repo instead
        GIT_REPOSITORY "https://github.com/Perlmint/glew-cmake.git"
        GIT_TAG "glew-cmake-1.11.0"
        CMAKE_ARGS
            -DONLY_LIBS=ON
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
)

set(GLEW_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}glew${CMAKE_SHARED_LIBRARY_SUFFIX})
list(APPEND LIBRARIES ${GLEW_LIBRARY})
list(APPEND FAST_EXTERNAL_DEPENDENCIES glew)
