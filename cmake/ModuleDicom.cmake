
if(FAST_MODULE_Dicom)
    message("-- Enabling dicom (DCMTK) module.")
    add_definitions("-DFAST_MODULE_DICOM")
    if(WIN32)
        fast_download_dependency(dcmtk
                3.6.3
                fbb84b29154fbf58833025188dea2c139caa2a5c136a5f48469839f43b9f6e05
                dcmdata.lib dcmimgle.lib ofstd.lib oflog.lib
        )
    else()
        fast_download_dependency(dcmtk
                3.6.3
                27e278ad3b3637548669c26fbefed365ae6211128b3dfbb7a35d46ed377a6f04
                libdcmdata.so libdcmimgle.so libofstd.so liboflog.so
        )
    endif()
endif()
