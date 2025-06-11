# Download M.CSS for building the documentation
# jinja2 is needed for python: pip3 install jinja2
include(${PROJECT_SOURCE_DIR}/../cmake/Externals.cmake)

ExternalProject_Add(mcss
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/mcss
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/mcss
        GIT_REPOSITORY "https://github.com/smistad/m.css"
        GIT_TAG "fast_constructor_hack"
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
)
