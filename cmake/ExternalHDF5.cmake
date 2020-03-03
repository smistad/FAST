# Download and set up HDF5 library

include(cmake/Externals.cmake)

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

if(WIN32)
  list(APPEND LIBRARIES hdf5.lib hdf5_cpp.lib)
else()
  list(APPEND LIBRARIES ${CMAKE_SHARED_LIBRARY_PREFIX}hdf5_cpp${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES hdf5)
