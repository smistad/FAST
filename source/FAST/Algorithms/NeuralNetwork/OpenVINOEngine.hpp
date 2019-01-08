#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>

namespace InferenceEngine {
class InferRequest;
}

namespace fast {

class OpenVINOEngine : public InferenceEngine {
    FAST_OBJECT(OpenVINOEngine)
    public:
        void run() override;

        void load() override;

        ImageOrdering getPreferredImageOrdering() const override;

        std::string getName() const override;

    private:
        std::shared_ptr<::InferenceEngine::InferRequest> m_inferRequest;
};

}