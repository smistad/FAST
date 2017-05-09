if(FAST_MODULE_NeuralNetwork)
    fast_add_sources(
        NeuralNetwork.cpp
        NeuralNetwork.hpp
        ImageClassifier.cpp
        ImageClassifier.hpp
        #DNNAppearanceModel.cpp
        #DNNAppearanceModel.hpp
        #ObjectDetection.cpp
        #ObjectDetection.hpp
        #ShapeRegressor.cpp
        #ShapeRegressor.hpp
            PixelClassifier.cpp
            PixelClassifier.hpp
    )
    fast_add_process_object(ImageClassifier ImageClassifier.hpp)
    fast_add_process_object(ClassificationToText ImageClassifier.hpp)
    fast_add_process_object(PixelClassifier PixelClassifier.hpp)
    if(FAST_MODULE_Visualization)
    fast_add_test_sources(
        ImageClassifierTests.cpp
        #DNNAppearanceModelTests.cpp
        ObjectDetectionTests.cpp
    )
    endif()
    fast_add_example(imageClassification imageClassification.cpp)
    fast_add_example(leftVentricleSegmentation leftVentricleSegmentation.cpp)
    fast_add_example(axillaryHeatmap axillaryHeatmap.cpp)
    if(FAST_MODULE_OpenIGTLink)
        #fast_add_example(arteryDetection arteryDetection.cpp)
        fast_add_example(streamingNeuralNetwork streamingNeuralNetwork.cpp)
    endif()
endif()