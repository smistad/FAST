## ITK
if(FAST_MODULE_ITK)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
    message("-- Enabling ITK interoperability")
    set(LIBRARIES ${LIBRARIES} ${ITK_LIBRARIES})
endif()