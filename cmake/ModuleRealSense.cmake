
if(FAST_MODULE_RealSense)
    message("-- Enabling real sense module.")
    include(cmake/ExternalRealSense.cmake)
endif()
