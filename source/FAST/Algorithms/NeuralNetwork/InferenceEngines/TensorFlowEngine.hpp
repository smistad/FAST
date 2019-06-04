#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>
#include <FAST/SmartPointers.hpp>
#include <TensorFlowExport.hpp>

// Forward declare
namespace tensorflow {
class Session;
}

namespace fast {

class INFERENCEENGINETENSORFLOW_EXPORT TensorFlowEngine : public InferenceEngine {
    FAST_OBJECT(TensorFlowEngine)
    public:
        void load() override;
        void run() override;
        ~TensorFlowEngine() override;
        ImageOrdering getPreferredImageOrdering() const override;
        std::string getName() const override;
        std::string getDefaultFileExtension() const override;
        TensorFlowEngine();
    private:
        std::unique_ptr<tensorflow::Session> mSession;
        std::vector<std::string> mLearningPhaseTensors;

};

DEFINE_INFERENCE_ENGINE(TensorFlowEngine, INFERENCEENGINETENSORFLOW_EXPORT)

}
