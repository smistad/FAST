# Download and set up LibJPEG library

include(cmake/Externals.cmake)

if(WIN32)
ExternalProject_Add(libjpeg
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/libjpeg
        URL "https://github.com/smistad/FAST-dependencies/releases/download/v4.0.0/LibJPEG-9d-Win-pc064.zip"
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/libjpeg/src/libjpeg/LibJPEG/9d/bin/libjpeg.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/libjpeg/src/libjpeg/LibJPEG/9d/lib/libjpeg.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/libjpeg/src/libjpeg/LibJPEG/9d/include/ ${FAST_EXTERNAL_INSTALL_DIR}/include/
    )
    list(APPEND LIBRARIES libjpeg.lib)
endif()
