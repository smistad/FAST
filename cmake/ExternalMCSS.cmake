# Download M.CSS for building the documentation
include(cmake/Externals.cmake)

ExternalProject_Add(mcss
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/mcss
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/mcss
        GIT_REPOSITORY "https://github.com/mosra/m.css"
        GIT_TAG "3a294739e2d351a40b94527eabd7c228d17b56be"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)