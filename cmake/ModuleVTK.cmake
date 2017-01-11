## VTK
if(FAST_MODULE_VTK)
    find_package(VTK REQUIRED)
    include(${VTK_USE_FILE})
    message("-- Enabling VTK interoperability. VTK version ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}")
    set(LIBRARIES ${LIBRARIES} ${VTK_LIBRARIES})
endif()
