# Download clarius listen API
include(cmake/Externals.cmake)

if(WIN32)
    set(URL "http://www.idi.ntnu.no/~smistad/clarius_listen_sdk_2019-10-16_21-18-03_isonosw.v6.0.0.windows.desktop_376_a62de5dadfb9338e2b541795b96eeb336a968026.zip")
else()
    set(URL "http://www.idi.ntnu.no/~smistad/clarius_listen_sdk_2019-10-16_15-23-46_isonosw.v6.0.0.linux.desktop_428_a62de5dadfb9338e2b541795b96eeb336a968026.zip")
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
