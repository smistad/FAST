# Download and set up GLEW

include(cmake/Externals.cmake)

ExternalProject_Add(glew
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/glew
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/glew
        # The original GLEW library of https://github.com/nigels-com/glew/ has not proper cmake setup
        # Use this repo instead
        GIT_REPOSITORY "https://github.com/Perlmint/glew-cmake.git"
        GIT_TAG "db1e93ef4c63f627ddc73cb12925a61d4f88e2b1"
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND}
		${FAST_EXTERNAL_BUILD_DIR}/glew/src/glew/build/cmake/
		-G${CMAKE_GENERATOR}
            -DBUILD_UTILS=OFF
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
)

if(WIN32)
set(GLEW_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/glew32.lib)
else()
set(GLEW_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}glew${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND LIBRARIES ${GLEW_LIBRARY})
list(APPEND FAST_EXTERNAL_DEPENDENCIES glew)
