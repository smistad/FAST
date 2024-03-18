# Download and set up OpenSlide

include(cmake/Externals.cmake)

if(WIN32)

ExternalProject_Add(openslide
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/openslide
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/openslide
        URL "https://github.com/openslide/openslide-bin/releases/download/v20231011/openslide-win64-20231011.zip"
        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/openslide/src/openslide/ ${FAST_EXTERNAL_INSTALL_DIR}
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)

list(APPEND FAST_EXTERNAL_DEPENDENCIES openslide)
endif()
