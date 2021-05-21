# Download and set up HDF5 library

include(cmake/Externals.cmake)

if(FAST_BUILD_ALL_DEPENDENCIES)
ExternalProject_Add(hdf5
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/hdf5
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/hdf5
        GIT_REPOSITORY "https://bitbucket.hdfgroup.org/scm/hdffv/hdf5.git"
        GIT_TAG "5b9cf732caab9daa6ed1e00f2df4f5a792340196" #"hdf5-1_10_6"
        UPDATE_COMMAND "" # Hack to avoid rebuild all the time on linux
        CMAKE_ARGS
          -DHDF5_NO_PACKAGES:BOOL=ON
          -DONLY_SHARED_LIBS:BOOL=ON
          -DBUILD_TESTING:BOOL=OFF
          -DHDF5_BUILD_TOOLS:BOOL=OFF
          -DHDF5_BUILD_EXAMPLES:BOOL=OFF
          -DHDF5_BUILD_CPP_LIB:BOOL=ON
          -DHDF5_BUILD_HL_LIB:BOOL=OFF
        CMAKE_CACHE_ARGS
          -DCMAKE_BUILD_TYPE:STRING=Release
          -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
          -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
          -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )
else(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
  set(FILENAME windows/hdf5_1.10.6_msvc14.2.tar.xz)
  set(SHA 02b52e481994850b762666870fac2979d5f8bff1d20e5f050fe3a8f200c44041)
else()
  set(FILENAME linux/hdf5_1.10.6_glibc2.27.tar.xz)
  set(SHA 640d2d924bb26a2f5bad5cfed62a54c724ec1130a0f4ffb163d2aa7de027ba74)
endif()
ExternalProject_Add(hdf5
      PREFIX ${FAST_EXTERNAL_BUILD_DIR}/hdf5
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
  list(APPEND LIBRARIES hdf5.lib hdf5_cpp.lib)
else()
  list(APPEND LIBRARIES ${CMAKE_SHARED_LIBRARY_PREFIX}hdf5_cpp${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES hdf5)
