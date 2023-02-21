if(FAST_MODULE_HDF5)
    message("-- Enabling HDF5 module")
    if(WIN32)
        fast_download_dependency(hdf5
                1.10.6
                495df6df0b742341ee385908a1a43558173a73f4b837e2457e7a9985d205f2c4
                hdf5.lib hdf5_cpp.lib
        )
    elseif(APPLE)
        if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
	fast_download_dependency(hdf5
                1.10.6
		1cb25e5add41c9ac4d201da1a0edecb0c962a7564da2fe941574c5fed2dba868
                libhdf5.dylib libhdf5_cpp.dylib
        )

	else()
        fast_download_dependency(hdf5
                1.10.6
                3dfeab5a3143c6f3452629936935810297eba010f706d047ce4da837193ccd2b
                libhdf5.dylib libhdf5_cpp.dylib
        )
endif()
    else()
        fast_download_dependency(hdf5
                1.10.6
                3582967e130f218861f896675dfe4ddc5807caf203237b99d597af9bf3e8e387
                libhdf5_cpp.so
        )
    endif()
endif()
