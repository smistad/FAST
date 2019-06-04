include(GenerateExportHeader)

if(FAST_MODULE_TensorFlow)
    include(${PROJECT_SOURCE_DIR}/cmake/ModuleTensorFlow.cmake)

    add_library(InferenceEngineTensorFlow SHARED TensorFlowEngine.hpp TensorFlowEngine.cpp)
    target_link_libraries(InferenceEngineTensorFlow ${TensorFlow_LIBRARIES})
    target_include_directories(InferenceEngineTensorFlow PRIVATE ${FAST_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
    generate_export_header(InferenceEngineTensorFlow EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/TensorFlowExport.hpp)
    add_dependencies(InferenceEngineTensorFlow tensorflow)

    fast_add_inference_engine(TensorFlow)
endif()
if(FAST_MODULE_TensorRT)
    include(${PROJECT_SOURCE_DIR}/cmake/ModuleTensorRT.cmake)

    add_library(InferenceEngineTensorRT SHARED TensorRTEngine.hpp TensorRTEngine.cpp)
    target_include_directories(InferenceEngineTensorRT PRIVATE ${FAST_INCLUDE_DIRS} ${TensorRT_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
    target_link_libraries(InferenceEngineTensorRT ${TensorRT_LIBRARIES})
    generate_export_header(InferenceEngineTensorRT EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/TensorRTExport.hpp)

    fast_add_inference_engine(TensorRT)
endif()
if(FAST_MODULE_OpenVINO)
    include(${PROJECT_SOURCE_DIR}/cmake/ModuleOpenVINO.cmake)

    add_library(InferenceEngineOpenVINO SHARED OpenVINOEngine.hpp OpenVINOEngine.cpp)
    target_include_directories(InferenceEngineOpenVINO PRIVATE ${FAST_INCLUDE_DIRS} ${InferenceEngine_INCLUDE_DIRS})
    target_link_libraries(InferenceEngineOpenVINO ${InferenceEngine_LIBRARIES} ${IntelTBB})
    generate_export_header(InferenceEngineOpenVINO EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/OpenVINOExport.hpp)
    add_dependencies(InferenceEngineOpenVINO IE::ie_cpu_extension)

    fast_add_inference_engine(OpenVINO)
endif()