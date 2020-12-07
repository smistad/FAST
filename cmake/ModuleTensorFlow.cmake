## TensorFlow module

if(FAST_MODULE_TensorFlow)
    message("-- TensorFlow module enabled. Select which TensorFlow versions to build (CPU only/CUDA/ROCM).")
    option(FAST_BUILD_TensorFlow_CUDA "Build TensorFlow CUDA/cuDNN version" OFF)
    if(NOT WIN32)
      option(FAST_BUILD_TensorFlow_ROCm "Build TensorFlow ROCm version" OFF)
    endif()
    include(${PROJECT_SOURCE_DIR}/cmake/ExternalTensorflow.cmake)

    ## Tensorflow
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_definitions(-DEIGEN_AVOID_STL_ARRAY)
    if(WIN32)
        set(TensorFlow_LIBRARIES
            ${FAST_EXTERNAL_INSTALL_DIR}/lib/tensorflow_cc.lib
        )
    else()
        set(TensorFlow_LIBRARIES
            ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc.so.2.3.0
        )
    endif()
endif()
