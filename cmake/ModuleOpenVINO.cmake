if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    include(${PROJECT_SOURCE_DIR}/cmake/ExternalOpenVINO.cmake)
endif()
