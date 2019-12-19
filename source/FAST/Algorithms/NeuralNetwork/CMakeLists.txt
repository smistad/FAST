fast_add_sources(
    NeuralNetwork.cpp
    NeuralNetwork.hpp
    ImageClassificationNetwork.cpp
    ImageClassificationNetwork.hpp
    SegmentationNetwork.cpp
    SegmentationNetwork.hpp
    ImageToImageNetwork.cpp
    ImageToImageNetwork.hpp
    InferenceEngine.cpp
    InferenceEngine.hpp
    InferenceEngineManager.cpp
    InferenceEngineManager.hpp
    TensorToSegmentation.cpp
    TensorToSegmentation.hpp
)
fast_add_process_object(ImageClassificationNetwork ImageClassificationNetwork.hpp)
fast_add_process_object(ClassificationToText ImageClassificationNetwork.hpp)
fast_add_process_object(SegmentationNetwork SegmentationNetwork.hpp)
fast_add_process_object(ImageToImageNetwork ImageToImageNetwork.hpp)
if(FAST_MODULE_Visualization)
    fast_add_test_sources(
        Tests.cpp
        ImageClassifierTests.cpp
    )
    if(FAST_MODULE_WholeSlideImaging)
        fast_add_test_sources(
            WSITests.cpp
            VolumeTests.cpp
        )
        fast_add_example(measureNeuralNetworkPerformance measureNeuralNetworkPerformance.cpp)
    endif()
endif()
#fast_add_example(timingNetwork timingNetwork.cpp)
#fast_add_example(imageClassification imageClassification.cpp)
#fast_add_example(leftVentricleSegmentation leftVentricleSegmentation.cpp)

fast_add_subdirectories(InferenceEngines)
