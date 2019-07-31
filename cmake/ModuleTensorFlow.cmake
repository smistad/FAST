## TensorFlow module

if(FAST_MODULE_TensorFlow)
    message("-- TensorFlow module enabled. Select which TensorFlow versions to build (CPU/CUDA/ROCM).")
    option(FAST_BUILD_TensorFlow_CUDA "Build TensorFlow CUDA/cuDNN version" OFF)
    option(FAST_BUILD_TensorFlow_CPU "Build TensorFlow CPU version" ON)
    if(NOT WIN32)
      option(FAST_BUILD_TensorFlow_ROCm "Build TensorFlow ROCm version" OFF)
    endif()
    include(${PROJECT_SOURCE_DIR}/cmake/ExternalTensorflow.cmake)

    ## Tensorflow
    #list(APPEND FAST_INCLUDE_DIRS ${Tensorflow_INCLUDE_DIRS})
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_definitions(-DEIGEN_AVOID_STL_ARRAY)
    if(WIN32)
    set(TensorFlow_CUDA_LIBRARIES
        ${FAST_EXTERNAL_INSTALL_DIR}/lib/tensorflow_CUDA.lib
    )
    set(TensorFlow_ROCm_LIBRARIES
            ${FAST_EXTERNAL_INSTALL_DIR}/lib/tensorflow_ROCm.lib
    )
    set(TensorFlow_CPU_LIBRARIES
        ${FAST_EXTERNAL_INSTALL_DIR}/lib/tensorflow_CPU.lib
    )
    else()
    set(TensorFlow_CUDA_LIBRARIES
        ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CUDA.so
    )
    set(TensorFlow_ROCm_LIBRARIES
            ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_ROCm.so
    )
    set(TensorFlow_CPU_LIBRARIES
        ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CPU.so
    )
    endif()
endif()
