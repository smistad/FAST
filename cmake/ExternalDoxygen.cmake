# Build and install a specific version of Doxygen for building the documentation
# FLEX and bison are required to build doxygen, see https://www.doxygen.nl/manual/install.html
include(${PROJECT_SOURCE_DIR}/../cmake/Externals.cmake)

ExternalProject_Add(doxygen
	PREFIX ${FAST_EXTERNAL_BUILD_DIR}/doxygen
        GIT_REPOSITORY "https://github.com/doxygen/doxygen.git"
	GIT_PROGRESS 1
	GIT_TAG "Release_1_8_18"
        CMAKE_ARGS
	    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
	    -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_BUILD_DIR}/doxygen/install/
)
