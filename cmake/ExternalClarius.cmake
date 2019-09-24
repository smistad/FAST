# Download clarius listen API
include(cmake/Externals.cmake)

if(WIN32)
    set(URL "http://www.idi.ntnu.no/~smistad/clarius_listen_sdk_2018-11-09_15-30-19_isonosw.v5.0.0.windows.desktop_80_f43778b091ea23a92c0f884a77e42fb65831e048.zip")
else()
    set(URL "http://www.idi.ntnu.no/~smistad/clarius_listen_sdk_2018-11-16_20-44-46_isonosw.v5.0.0.linux.desktop_80_1cf035e5a38bb0ba4ddde2b35c54024b1d4230d1.zip")
endif()

if(WIN32)
ExternalProject_Add(clarius
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/clarius # The folder in which the package will downloaded to
        URL ${URL}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/lib/listen.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
          ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/lib/listen.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
          ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/include ${FAST_EXTERNAL_INSTALL_DIR}/include/
)
else()
ExternalProject_Add(clarius
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/clarius # The folder in which the package will downloaded to
        URL ${URL}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/include ${FAST_EXTERNAL_INSTALL_DIR}/include/
)
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES clarius)
