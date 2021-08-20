if(FAST_MODULE_HDF5)
    message("-- Enabling HDF5 module")
    if(WIN32)
        fast_download_dependency(hdf5
                1.10.6
                495df6df0b742341ee385908a1a43558173a73f4b837e2457e7a9985d205f2c4
                hdf5.lib hdf5_cpp.lib
        )
    else()
        fast_download_dependency(hdf5
                1.10.6
                0cc43f23e7f0ee29fd9be5d65b0b4e8dfbe4b2d3192ba0686f5be8dc2e7883cc
                libhdf5_cpp.so
        )
    endif()
endif()
