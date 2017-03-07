
if(FAST_MODULE_Kinect)
    message("-- Enabling Kinect module.")
    include(cmake/ExternalFreenect2.cmake)
endif()