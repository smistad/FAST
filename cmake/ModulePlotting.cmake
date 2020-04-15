if(FAST_MODULE_Plotting)
    message("-- Enabling plotting module")
    include(${PROJECT_SOURCE_DIR}/cmake/ExternalJKQTPlotter.cmake)
endif()
