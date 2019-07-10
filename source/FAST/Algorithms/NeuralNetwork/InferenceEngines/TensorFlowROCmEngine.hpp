#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngines/TensorFlowEngine.hpp>
#include <TensorFlowROCmExport.hpp>

namespace fast {

class INFERENCEENGINETENSORFLOWCUDA_EXPORT TensorFlowROCmEngine : public TensorFlowEngine {
    FAST_OBJECT(TensorFlowROCmEngine)
    public:
        std::string getName() const override { return "TensorFlowROCm"; };

};

DEFINE_INFERENCE_ENGINE(TensorFlowROCmEngine, INFERENCEENGINETENSORFLOWROCM_EXPORT)

}
