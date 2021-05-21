# Download and set up libfreenect2

include(cmake/Externals.cmake)

if(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
ExternalProject_Add(realsense
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/realsense
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/realsense
        GIT_REPOSITORY "https://github.com/IntelRealSense/librealsense.git"
        GIT_TAG "v2.40.0"
        CMAKE_ARGS
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_GRAPHICAL_EXAMPLES:BOOL=OFF
        -DBUILD_EASYLOGGINGPP:BOOL=OFF
        -DBUILD_WITH_TM2:BOOL=OFF
        -DBUILD_WITH_OPENMP:BOOL=ON
        CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
        -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
        -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )
else()
ExternalProject_Add(realsense
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/realsense
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/realsense
        GIT_REPOSITORY "https://github.com/IntelRealSense/librealsense.git"
        GIT_TAG "v2.40.0"
	INSTALL_COMMAND make install/strip
        CMAKE_ARGS
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_GRAPHICAL_EXAMPLES:BOOL=OFF
        -DBUILD_EASYLOGGINGPP:BOOL=OFF
        -DBUILD_WITH_TM2:BOOL=OFF
        -DBUILD_WITH_OPENMP:BOOL=ON
        CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=Release
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
        -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
        -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )
endif()
else(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
  set(FILENAME windows/realsense_2.40.0_msvc14.2.tar.xz)
  set(SHA 1315199e6f89ffa4e8038f5d1326b7ff2470fbdd5d1f9f0b62ffaa85b47c1297)
else()
  set(FILENAME linux/realsense_2.40.0_glibc2.27.tar.xz)
  set(SHA a68b5857f255289993dde6b0b9d5b87e19a9dbb980e11ea7989dd3d7ae03adab)
endif()
ExternalProject_Add(realsense
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/realsense
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

list(APPEND LIBRARIES ${CMAKE_SHARED_LIBRARY_PREFIX}realsense2${CMAKE_SHARED_LIBRARY_SUFFIX})
list(APPEND FAST_EXTERNAL_DEPENDENCIES realsense)
