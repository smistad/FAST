if(FAST_MODULE_TensorRT)
    message("-- Enabling TensorRT inference engine module")
    find_package(TensorRT REQUIRED)

    list(APPEND FAST_INCLUDE_DIRS ${TensorRT_INCLUDE_DIRS})
        message("-- trt lib: ${TensorRT_LIBRARIES}")
    list(APPEND LIBRARIES ${TensorRT_LIBRARIES})

    find_package(CUDA REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS ${CUDA_INCLUDE_DIRS})
endif()