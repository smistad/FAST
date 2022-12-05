#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>
#include <TensorRTExport.hpp>

namespace nvinfer1 {
class ICudaEngine;
class IExecutionContext;
}

namespace fast {

class INFERENCEENGINETENSORRT_EXPORT TensorRTEngine : public InferenceEngine {
    FAST_OBJECT(TensorRTEngine)
    public:
        void run() override;
        void load() override;
        ImageOrdering getPreferredImageOrdering() const override;
        std::string getName() const override;
		std::vector<ModelFormat> getSupportedModelFormats() const {
            return { ModelFormat::ONNX, ModelFormat::UFF };
        };
        ModelFormat getPreferredModelFormat() const {
            return ModelFormat::ONNX;
        };
        ~TensorRTEngine() override;
        void setMaxBatchSize(int maxBathSize);
        int getMaxBatchSize() const;
        TensorRTEngine();
    private:
        nvinfer1::ICudaEngine* m_engine = nullptr;
        nvinfer1::IExecutionContext* m_context = nullptr;
        std::size_t m_maxWorkspaceSize = 1024*1024*1024; // in bytes
        std::vector<void*> m_cudaBuffers;
        std::map<std::string, int> m_inputIndexes;
        std::map<std::string, int> m_outputIndexes;
        int m_currentBatchSize = 1;
};

DEFINE_INFERENCE_ENGINE(TensorRTEngine, INFERENCEENGINETENSORRT_EXPORT)

}