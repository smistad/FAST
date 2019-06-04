# OpenMP

find_package(OpenMP)
if(OPENMP_FOUND)
    message("-- OpenMP was detected. Using OpenMP to speed up some calculations.")
    set_target_properties(FAST PROPERTIES CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
    set_target_properties(FAST PROPERTIES CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
    set_target_properties(FAST PROPERTIES CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" )
endif()