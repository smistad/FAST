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
        std::string getDefaultFileExtension() const override;
        ~TensorRTEngine() override;
        void setMaxBatchSize(int maxBathSize);
        int getMaxBatchSize() const;
        TensorRTEngine();
    private:
        nvinfer1::ICudaEngine* m_engine = nullptr;
        nvinfer1::IExecutionContext* m_context = nullptr;
        int m_maxBatchSize = 1;
        std::size_t m_maxWorkspaceSize = 4*1024*1024; // in bytes
};

#define ASD
DEFINE_INFERENCE_ENGINE(TensorRTEngine, ASD)

}