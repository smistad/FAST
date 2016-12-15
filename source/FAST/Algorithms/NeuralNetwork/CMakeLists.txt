if(FAST_MODULE_NeuralNetwork)
    fast_add_sources(
        NeuralNetwork.cpp
        NeuralNetwork.hpp
        ImageClassifier.cpp
        ImageClassifier.hpp
        #DNNAppearanceModel.cpp
        #DNNAppearanceModel.hpp
        ObjectDetection.cpp
        ObjectDetection.hpp
    )
    fast_add_test_sources(
        ImageClassifierTests.cpp
        #DNNAppearanceModelTests.cpp
        ObjectDetectionTests.cpp
    )
    fast_add_example(imageClassification imageClassification.cpp)
    if(FAST_MODULE_OpenIGTLink)
        #fast_add_example(arteryDetection arteryDetection.cpp)
    endif()
endif()