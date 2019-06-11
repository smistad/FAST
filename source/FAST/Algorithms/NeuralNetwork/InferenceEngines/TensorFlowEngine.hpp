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
    public:
        virtual void load() override;
        virtual void run() override;
        virtual std::string getName() const = 0;
        ~TensorFlowEngine() override;
        virtual ImageOrdering getPreferredImageOrdering() const override;
        virtual std::string getDefaultFileExtension() const override;
        TensorFlowEngine();
    protected:
        std::unique_ptr<tensorflow::Session> mSession;
        std::vector<std::string> mLearningPhaseTensors;

};


}
