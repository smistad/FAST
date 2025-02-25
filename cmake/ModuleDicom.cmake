
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
                3.6.7
		c2a56ed04763a5d542500a22dc8f77d61d2050ef951d1ad0915abbcea957e82d
                libdcmimage.dylib libdcmjpeg.dylib libdcmdata.dylib libdcmimgle.dylib libofstd.dylib liboflog.dylib
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
                3.6.3
                f40ba5df0307c0ac20a200c2384835c404adb3341c44cd67201946bd4c9006d6
                libdcmdata.so libdcmimgle.so libofstd.so liboflog.so
        )
    endif()
endif()
