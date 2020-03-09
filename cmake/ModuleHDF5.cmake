if(FAST_MODULE_HDF5)
    message("-- Enabling HDF5 module")
    include(${PROJECT_SOURCE_DIR}/cmake/ExternalHDF5.cmake)
endif()
