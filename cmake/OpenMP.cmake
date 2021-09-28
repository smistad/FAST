# OpenMP

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("-- OpenMP was detected. Using OpenMP to speed up some calculations.")
    list(APPEND FAST_INCLUDE_DIRS ${OpenMP_CXX_INCLUDE_DIR})
    list(APPEND LIBRARIES ${OpenMP_libomp_LIBRARY})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" )
endif()
