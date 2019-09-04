if(FAST_MODULE_OpenVINO)
    message("-- Enabling Intel OpenVINO inference engine module")
    if(WIN32)
        set(InferenceEngine_BASE "C:/Program Files (x86)/IntelSWTools/openvino/inference_engine")
        set(InferenceEngine_DIR ${InferenceEngine_BASE}/share)
        find_package(InferenceEngine REQUIRED)

        # Avoid some OpenMP issues with the ie_cpu_extensions target
        set_target_properties(ie_cpu_extension PROPERTIES CMAKE_C_FLAGS "")
        set_target_properties(ie_cpu_extension PROPERTIES CMAKE_CXX_FLAGS "")
        set_target_properties(ie_cpu_extension PROPERTIES CMAKE_EXE_LINKER_FLAGS "")
    else()
        include(${PROJECT_SOURCE_DIR}/cmake/ExternalOpenVINO.cmake)
    endif()


endif()
