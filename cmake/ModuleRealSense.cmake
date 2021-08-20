
if(FAST_MODULE_RealSense)
    message("-- Enabling real sense module.")
    if(WIN32)
        fast_download_dependency(realsense
                2.40.0
                f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
                realsense2.lib
        )
    else()
        fast_download_dependency(realsense
                2.40.0
                19ab75148489d860ec2b73ad5edaad9000a6d155efb90ce6b4603aaf355bcd74
                librealsense2.so
        )
    endif()
endif()
