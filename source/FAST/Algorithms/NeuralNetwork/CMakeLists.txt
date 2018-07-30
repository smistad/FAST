if(FAST_MODULE_NeuralNetwork)
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
    #fast_add_example(imageClassification imageClassification.cpp)
    #fast_add_example(leftVentricleSegmentation leftVentricleSegmentation.cpp)
    #fast_add_example(axillaryHeatmap axillaryHeatmap.cpp)
endif()