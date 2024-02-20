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
                1.10.11
  		eb9cac9cf9b49b35b7681712d3f1a6bb12ea0b54cc772bcd468f812256ce51ef
                libhdf5.a libhdf5_cpp.a
        )

	else()
        fast_download_dependency(hdf5
                1.10.11
		97b7fe90182293ad137a8f0b9e7a9c3752ba910966f931da163698de60acffad
                libhdf5.a libhdf5_cpp.a
        )
endif()
    else()
        fast_download_dependency(hdf5
                1.10.11
                b4b937d69cc475f7304a2e0d3e44ed74ce1ae8130649d451eaabcdc171cc9077
                libhdf5.a libhdf5_cpp.a
        )
    endif()
endif()
