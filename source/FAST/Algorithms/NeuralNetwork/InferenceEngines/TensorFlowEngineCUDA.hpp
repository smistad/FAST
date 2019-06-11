#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngines/TensorFlowEngine.hpp>

namespace fast {

class INFERENCEENGINETENSORFLOWCUDA_EXPORT TensorFlowCUDAEngine : public InferenceEngine {
    FAST_OBJECT(TensorFlowEngineCUDA)
    public:
        std::string getName() const override { return "TensorFlowCUDA"; };

};

DEFINE_INFERENCE_ENGINE(TensorFlowCUDAEngine, INFERENCEENGINETENSORFLOWCUDA_EXPORT)

}
