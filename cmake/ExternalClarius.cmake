# Download clarius cast API
include(cmake/Externals.cmake)

# Download license
file(DOWNLOAD https://raw.githubusercontent.com/clariusdev/cast/v11.2.0/LICENSE ${FAST_EXTERNAL_INSTALL_DIR}/licenses/clarius/LICENSE)

if(WIN32)
    set(URL "https://github.com/clariusdev/cast/releases/download/v11.2.0/cast-11.2.0-windows.x86_64.zip")
elseif(APPLE)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(URL "https://github.com/clariusdev/cast/releases/download/v11.2.0/cast-11.2.0-macos.arm64.zip")
    else()
        set(URL "https://github.com/clariusdev/cast/releases/download/v11.2.0/cast-11.2.0-macos.x86_64.zip")
    endif()
else()
    set(URL "https://github.com/clariusdev/cast/releases/download/v11.2.0/cast-11.2.0-linux.x86_64-gcc_ubuntu_20.04.zip")
endif()

if(WIN32)
ExternalProject_Add(clarius
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/clarius # The folder in which the package will downloaded to
        URL ${URL}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/cast.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
          ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/cast.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
          ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/include ${FAST_EXTERNAL_INSTALL_DIR}/include/
)
elseif(APPLE)
if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    ExternalProject_Add(clarius
          PREFIX ${FAST_EXTERNAL_BUILD_DIR}/clarius # The folder in which the package will downloaded to
          URL ${URL}
          UPDATE_COMMAND ""
          CONFIGURE_COMMAND ""
          BUILD_COMMAND ""
          INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/libcast${CMAKE_SHARED_LIBRARY_SUFFIX} ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
              ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/include ${FAST_EXTERNAL_INSTALL_DIR}/include/
    )
else()
    ExternalProject_Add(clarius
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/clarius # The folder in which the package will downloaded to
            URL ${URL}
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/libcast${CMAKE_SHARED_LIBRARY_SUFFIX} ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/include ${FAST_EXTERNAL_INSTALL_DIR}/include/
    )
endif()
else()
ExternalProject_Add(clarius
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/clarius # The folder in which the package will downloaded to
        URL ${URL}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/libcast${CMAKE_SHARED_LIBRARY_SUFFIX} ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/include ${FAST_EXTERNAL_INSTALL_DIR}/include/
)
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES clarius)
