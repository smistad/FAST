    fast_add_sources(
        NeuralNetwork.cpp
        NeuralNetwork.hpp
        ImageClassifier.cpp
        ImageClassifier.hpp
        PixelClassifier.cpp
        PixelClassifier.hpp
        ImageToImageNetwork.cpp
        ImageToImageNetwork.hpp
        InferenceEngine.cpp
        InferenceEngine.hpp
        InferenceEngineManager.cpp
        InferenceEngineManager.hpp
    )
    fast_add_process_object(ImageClassifier ImageClassifier.hpp)
    fast_add_process_object(ClassificationToText ImageClassifier.hpp)
    fast_add_process_object(PixelClassifier PixelClassifier.hpp)
    fast_add_process_object(ImageToImageNetwork ImageToImageNetwork.hpp)
    if(FAST_MODULE_Visualization)
    fast_add_test_sources(
        Tests.cpp
        ImageClassifierTests.cpp
        VolumeTests.cpp
    )
        if(FAST_MODULE_WholeSlideImaging)
            fast_add_test_sources(WSITests.cpp)
            fast_add_example(measureNeuralNetworkPerformance measureNeuralNetworkPerformance.cpp)
        endif()
    endif()
    #fast_add_example(timingNetwork timingNetwork.cpp)
    #fast_add_example(imageClassification imageClassification.cpp)
    #fast_add_example(leftVentricleSegmentation leftVentricleSegmentation.cpp)

fast_add_subdirectories(InferenceEngines)
