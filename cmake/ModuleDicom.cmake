
if(FAST_MODULE_Dicom)
    message("-- Enabling dicom (DCMTK) module.")
    add_definitions("-DFAST_MODULE_DICOM")
    if(WIN32)
        fast_download_dependency(dcmtk
                3.6.3
                f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
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
