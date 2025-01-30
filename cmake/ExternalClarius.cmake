# Download clarius cast API
include(cmake/Externals.cmake)

set(CLARIUS_CAST_VERSION "12.0.0")

# Download license
file(DOWNLOAD https://raw.githubusercontent.com/clariusdev/cast/v${CLARIUS_CAST_VERSION}/LICENSE ${FAST_EXTERNAL_INSTALL_DIR}/licenses/clarius/LICENSE)

if(WIN32)
    set(URL "https://github.com/clariusdev/cast/releases/download/v${CLARIUS_CAST_VERSION}/cast-${CLARIUS_CAST_VERSION}-windows.x86_64.zip")
elseif(APPLE)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(URL "https://github.com/clariusdev/cast/releases/download/v${CLARIUS_CAST_VERSION}/cast-${CLARIUS_CAST_VERSION}-macos.arm64.zip")
    else()
        set(URL "https://github.com/clariusdev/cast/releases/download/v${CLARIUS_CAST_VERSION}/cast-${CLARIUS_CAST_VERSION}-macos.x86_64.zip")
    endif()
else()
    set(URL "https://github.com/clariusdev/cast/releases/download/v${CLARIUS_CAST_VERSION}/cast-${CLARIUS_CAST_VERSION}-linux.x86_64-gcc_ubuntu_22.04.zip")
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
          ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/cast ${FAST_EXTERNAL_INSTALL_DIR}/include/cast
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
              ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/cast ${FAST_EXTERNAL_INSTALL_DIR}/include/cast
    )
else()
    ExternalProject_Add(clarius
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/clarius # The folder in which the package will downloaded to
            URL ${URL}
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/libcast${CMAKE_SHARED_LIBRARY_SUFFIX} ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/cast ${FAST_EXTERNAL_INSTALL_DIR}/include/cast
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
            ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/clarius/src/clarius/cast ${FAST_EXTERNAL_INSTALL_DIR}/include/cast
)
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES clarius)
