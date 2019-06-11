#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngines/TensorFlowEngine.hpp>
#include <TensorFlowCPUExport.hpp>

namespace fast {

class INFERENCEENGINETENSORFLOWCPU_EXPORT TensorFlowCPUEngine : public InferenceEngine {
    FAST_OBJECT(TensorFlowEngineCPU)
    public:
        std::string getName() const override { return "TensorFlowCPU"; };

};

DEFINE_INFERENCE_ENGINE(TensorFlowCPUEngine, INFERENCEENGINETENSORFLOWCPU_EXPORT)

}
