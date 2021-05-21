# Download and set up OpenIGTLink

include(cmake/Externals.cmake)

if(FAST_BUILD_ALL_DEPENDENCIES)
ExternalProject_Add(OpenIGTLink
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/OpenIGTLink
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/OpenIGTLink
        GIT_REPOSITORY "https://github.com/openigtlink/OpenIGTLink.git"
        GIT_TAG "v2.1"
        CMAKE_ARGS
            -DBUILD_SHARED_LIBS=ON
            -DBUILD_TESTING=OFF
            -DBUILD_EXAMPLES=OFF
            -DLIBRARY_OUTPUT_PATH=${FAST_EXTERNAL_INSTALL_DIR}/lib/
            -DCMAKE_MACOSX_RPATH=ON
            -DOpenIGTLink_INSTALL_LIB_DIR=${FAST_EXTERNAL_INSTALL_DIR}/lib/
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
)
else(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
  set(FILENAME windows/openigtlink_2.1.0_msvc14.2.tar.xz)
  set(SHA c93fec0ba9fc6a20b321f79b59af2af82b2364d3e54b33c6967ee81f09b50e0a)
else()
  set(FILENAME linux/openigtlink_2.1.0_glibc2.27.tar.xz)
  set(SHA 15c1987df275d26628f36fe25b316bad097f0903f5dfe1891db362e4926d7e29)
endif()
ExternalProject_Add(OpenIGTLink
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/OpenIGTLink
        URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL}/${FILENAME}
        URL_HASH SHA256=${SHA}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        # On install: Copy contents of each subfolder to the build folder
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${FAST_EXTERNAL_INSTALL_DIR}/include COMMAND
            ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin ${FAST_EXTERNAL_INSTALL_DIR}/bin COMMAND
            ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib COMMAND
            ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licences ${FAST_EXTERNAL_INSTALL_DIR}/licences
)
endif(FAST_BUILD_ALL_DEPENDENCIES)

if(WIN32)
set(OpenIGTLink_LIBRARY OpenIGTLink.lib)
else()
set(OpenIGTLink_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}OpenIGTLink${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND LIBRARIES ${OpenIGTLink_LIBRARY})
list(APPEND FAST_EXTERNAL_DEPENDENCIES OpenIGTLink)
