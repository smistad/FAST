    fast_add_sources(
        NeuralNetwork.cpp
        NeuralNetwork.hpp
        ImageClassifier.cpp
        ImageClassifier.hpp
        #ObjectDetection.cpp
        #ObjectDetection.hpp
        #ShapeRegressor.cpp
        #ShapeRegressor.hpp
        PixelClassifier.cpp
        PixelClassifier.hpp
        ImageToImageNetwork.cpp
        ImageToImageNetwork.hpp
        InferenceEngine.cpp
        InferenceEngine.hpp
        InferenceEngineRegistry.hpp
    )
    fast_add_process_object(ImageClassifier ImageClassifier.hpp)
    fast_add_process_object(ClassificationToText ImageClassifier.hpp)
    fast_add_process_object(PixelClassifier PixelClassifier.hpp)
    fast_add_process_object(ImageToImageNetwork ImageToImageNetwork.hpp)
    if(FAST_MODULE_Visualization)
    fast_add_test_sources(
        Tests.cpp
        ImageClassifierTests.cpp
        ObjectDetectionTests.cpp
    )
    endif()
    #fast_add_example(timingNetwork timingNetwork.cpp)
    #fast_add_example(imageClassification imageClassification.cpp)
    #fast_add_example(leftVentricleSegmentation leftVentricleSegmentation.cpp)
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