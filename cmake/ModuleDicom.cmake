
if(FAST_MODULE_Dicom)
    message("-- Enabling dicom (DCMTK) module.")
    add_definitions("-DFAST_MODULE_DICOM")
    if(WIN32)
        fast_download_dependency(dcmtk
                3.6.7
                37cd22205fb2e07f687be5f1d27d4e8ffdf29f01a87ffa3fca6ad86f00d95909
                dcmimage.lib dcmjpeg.lib dcmdata.lib dcmimgle.lib ofstd.lib oflog.lib
        )
    elseif(APPLE)
	if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        fast_download_dependency(dcmtk
                3.6.9
		        5f826356a67133dc29b18b4b10000bed566b1175f0f3291be8d913849a6d07c6
                libdcmimage.dylib libdcmjpeg.dylib libdcmdata.dylib libdcmimgle.dylib libofstd.dylib liboflog.dylib liboficonv.dylib
                )
	else()
        fast_download_dependency(dcmtk
                3.6.7
                308081dcf9622234263c9c7312450dbb8d54285224ad750f3d4f07aa48558b74
                libdcmimage.dylib libdcmjpeg.dylib libdcmdata.dylib libdcmimgle.dylib libofstd.dylib liboflog.dylib
          )
	endif()
    else()
        fast_download_dependency(dcmtk
                3.6.7
                e681ed35c487f24de9fe2845b261bda8d0024a95a714f311f128c78ee164dd4b
                libdcmimage.so libdcmjpeg.so libdcmdata.so libdcmimgle.so libofstd.so liboflog.so
        )
    endif()
endif()
