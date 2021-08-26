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
                3582967e130f218861f896675dfe4ddc5807caf203237b99d597af9bf3e8e387
                libhdf5_cpp.so
        )
    endif()
endif()
