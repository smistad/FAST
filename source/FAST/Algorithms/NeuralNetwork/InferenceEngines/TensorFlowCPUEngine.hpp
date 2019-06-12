#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngines/TensorFlowEngine.hpp>
#include <TensorFlowCPUExport.hpp>

namespace fast {

class INFERENCEENGINETENSORFLOWCPU_EXPORT TensorFlowCPUEngine : public TensorFlowEngine {
    FAST_OBJECT(TensorFlowCPUEngine)
    public:
        std::string getName() const override { return "TensorFlowCPU"; };

};

DEFINE_INFERENCE_ENGINE(TensorFlowCPUEngine, INFERENCEENGINETENSORFLOWCPU_EXPORT)

}
