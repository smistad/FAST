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
fast_add_python_interfaces(InferenceEngine.hpp InferenceEngineManager.hpp)
fast_add_python_shared_pointers(InferenceEngine)
fast_add_process_object(NeuralNetwork NeuralNetwork.hpp)
fast_add_process_object(ImageClassificationNetwork ImageClassificationNetwork.hpp)
fast_add_python_shared_pointers(ImageClassification)
fast_add_process_object(ClassificationToText ImageClassificationNetwork.hpp)
fast_add_process_object(SegmentationNetwork SegmentationNetwork.hpp)
fast_add_process_object(BoundingBoxNetwork BoundingBoxNetwork.hpp)
fast_add_process_object(FlowNetwork FlowNetwork.hpp)
fast_add_process_object(ImagesToSequence FlowNetwork.hpp)
fast_add_process_object(ImageToImageNetwork ImageToImageNetwork.hpp)
fast_add_process_object(TensorToSegmentation TensorToSegmentation.hpp)
fast_add_process_object(TensorToImage TensorToImage.hpp)
fast_add_process_object(TensorToBoundingBoxSet TensorToBoundingBoxSet.hpp)
fast_add_process_object(VertexTensorToSegmentation VertexTensorToSegmentation.hpp)

if(FAST_MODULE_Visualization)
    fast_add_test_sources(
        Tests.cpp
    )
endif()

fast_add_subdirectories(InferenceEngines)
