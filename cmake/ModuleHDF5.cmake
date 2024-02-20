if(FAST_MODULE_HDF5)
    message("-- Enabling HDF5 module")
    if(WIN32)
        fast_download_dependency(hdf5
                1.10.11
                affbe4c3ad9f5467ac8940f53da0d5196450fd1def46ef2cc4d009cd1834d5ce
                libhdf5.lib libhdf5_cpp.lib
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
