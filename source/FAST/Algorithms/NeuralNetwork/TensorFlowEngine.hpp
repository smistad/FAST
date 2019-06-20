#pragma once

#include "InferenceEngine.hpp"
#include <FAST/SmartPointers.hpp>

// Forward declare
namespace tensorflow {
class Session;
}

namespace fast {

class FAST_EXPORT TensorFlowEngine : public InferenceEngine {
    FAST_OBJECT(TensorFlowEngine)
    public:
        void load() override;
        void run() override;
        ~TensorFlowEngine() override;
        ImageOrdering getPreferredImageOrdering() const override;
        std::string getName() const override;
        std::string getDefaultFileExtension() const override;
    private:
        TensorFlowEngine();
        std::unique_ptr<tensorflow::Session> mSession;
        std::vector<std::string> mLearningPhaseTensors;

};

}
