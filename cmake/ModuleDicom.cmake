
if(FAST_MODULE_Dicom)
    message("-- Enabling dicom (DCMTK) module.")
    add_definitions("-DFAST_MODULE_DICOM")
    include(cmake/ExternalDCMTK.cmake)
endif()
