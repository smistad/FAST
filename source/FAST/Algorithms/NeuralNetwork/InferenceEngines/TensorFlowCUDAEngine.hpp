#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngines/TensorFlowEngine.hpp>
#include <TensorFlowCUDAExport.hpp>

namespace fast {

class INFERENCEENGINETENSORFLOWCUDA_EXPORT TensorFlowCUDAEngine : public TensorFlowEngine {
    FAST_OBJECT(TensorFlowCUDAEngine)
    public:
        std::string getName() const override { return "TensorFlowCUDA"; };

};

DEFINE_INFERENCE_ENGINE(TensorFlowCUDAEngine, INFERENCEENGINETENSORFLOWCUDA_EXPORT)

}
