#pragma once

#include "InferenceEngine.hpp"

namespace nvinfer1 {
class ICudaEngine;
class IExecutionContext;
}

namespace fast {

class TensorRTEngine : public InferenceEngine {
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
    private:
        TensorRTEngine();
        nvinfer1::ICudaEngine* m_engine = nullptr;
        nvinfer1::IExecutionContext* m_context = nullptr;
        int m_maxBatchSize = 1;
        std::size_t m_maxWorkspaceSize = 4*1024*1024; // in bytes
};

}