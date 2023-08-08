# OpenMP

find_package(OpenMP)
if(OPENMP_FOUND)
    message("-- OpenMP was detected. Using OpenMP to speed up some calculations.")
    list(APPEND FAST_INCLUDE_DIRS ${OpenMP_CXX_INCLUDE_DIR})
    list(APPEND LIBRARIES ${OpenMP_libomp_LIBRARY})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" )
else()
    if(APPLE)
        # libomp 15.0+ from brew is keg-only, so have to search in other locations.
        # See https://github.com/Homebrew/homebrew-core/issues/112107#issuecomment-1278042927.
        execute_process(COMMAND brew --prefix libomp
                OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
        set(OpenMP_C_LIB_NAMES omp)
        set(OpenMP_CXX_LIB_NAMES omp)
        set(OpenMP_omp_LIBRARY ${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib)
        find_package(OpenMP REQUIRED)
    endif()
endif()
