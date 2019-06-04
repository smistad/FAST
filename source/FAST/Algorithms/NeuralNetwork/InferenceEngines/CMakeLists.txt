if(FAST_MODULE_TensorFlow)
    fast_add_sources(
        TensorFlowEngine.cpp
        TensorFlowEngine.hpp
    )
    fast_add_inference_engine(TensorFlow TensorFlowEngine.hpp)
endif()
if(FAST_MODULE_TensorRT)
    fast_add_sources(
        TensorRTEngine.cpp
        TensorRTEngine.hpp
    )
    fast_add_inference_engine(TensorRT TensorRTEngine.hpp)
endif()
if(FAST_MODULE_OpenVINO)
    fast_add_sources(
        OpenVINOEngine.cpp
        OpenVINOEngine.hpp
    )
    fast_add_inference_engine(OpenVINO OpenVINOEngine.hpp)
endif()