
if(FAST_MODULE_RealSense)
    message("-- Enabling real sense module.")
    if(WIN32)
        fast_download_dependency(realsense
                2.40.0
                f13f1435ef9498b0b53c869c2f389b66bcae220591625ab6059319a0c4acfa2e
                realsense2.lib
        )
    else()
        fast_download_dependency(realsense
                2.40.0
                a18181ea9c4ce56e117882f441c59f528fca9181087da9ac29e870046dbd2906
                librealsense2.so
        )
    endif()
endif()
