if(FAST_MODULE_HDF5)
    message("-- Enabling HDF5 module")
    if(WIN32)
        fast_download_dependency(hdf5
                1.10.6
                f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
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
