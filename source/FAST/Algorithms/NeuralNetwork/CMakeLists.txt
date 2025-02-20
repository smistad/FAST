fast_add_sources(
    NeuralNetwork.cpp
    NeuralNetwork.hpp
    ImageClassificationNetwork.cpp
    ImageClassificationNetwork.hpp
    SegmentationNetwork.cpp
    SegmentationNetwork.hpp
    InferenceEngine.cpp
    InferenceEngine.hpp
    InferenceEngineManager.cpp
    InferenceEngineManager.hpp
    TensorToSegmentation.cpp
    TensorToSegmentation.hpp
    TensorToImage.cpp
    TensorToImage.hpp
    TensorToBoundingBoxSet.cpp
    TensorToBoundingBoxSet.hpp
    BoundingBoxNetwork.cpp
    BoundingBoxNetwork.hpp
    FlowNetwork.cpp
    FlowNetwork.hpp
    ImageToImageNetwork.cpp
    ImageToImageNetwork.hpp
    VertexTensorToSegmentation.cpp
    VertexTensorToSegmentation.hpp
)
fast_add_python_interfaces(ALGORITHMS InferenceEngine.hpp InferenceEngineManager.hpp)
fast_add_python_shared_pointers(ALGORITHMS InferenceEngine)
fast_add_process_object(ALGORITHMS NeuralNetwork NeuralNetwork.hpp)
fast_add_process_object(ALGORITHMS ImageClassificationNetwork ImageClassificationNetwork.hpp)
fast_add_python_shared_pointers(ALGORITHMS ImageClassification)
fast_add_process_object(ALGORITHMS ClassificationToText ImageClassificationNetwork.hpp)
fast_add_process_object(ALGORITHMS SegmentationNetwork SegmentationNetwork.hpp)
fast_add_process_object(ALGORITHMS BoundingBoxNetwork BoundingBoxNetwork.hpp)
fast_add_process_object(ALGORITHMS FlowNetwork FlowNetwork.hpp)
fast_add_process_object(ALGORITHMS ImagesToSequence FlowNetwork.hpp)
fast_add_process_object(ALGORITHMS ImageToImageNetwork ImageToImageNetwork.hpp)
fast_add_process_object(ALGORITHMS TensorToSegmentation TensorToSegmentation.hpp)
fast_add_process_object(ALGORITHMS TensorToImage TensorToImage.hpp)
fast_add_process_object(ALGORITHMS TensorToBoundingBoxSet TensorToBoundingBoxSet.hpp)
fast_add_process_object(ALGORITHMS VertexTensorToSegmentation VertexTensorToSegmentation.hpp)

if(FAST_MODULE_Visualization)
    fast_add_test_sources(
        Tests.cpp
    )
endif()

fast_add_subdirectories(InferenceEngines)
