#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>

namespace InferenceEngine {
class InferRequest;
class InferencePlugin;
}

namespace fast {

class OpenVINOEngine : public InferenceEngine {
    FAST_OBJECT(OpenVINOEngine)
    public:
        void run() override;

        void load() override;

        ImageOrdering getPreferredImageOrdering() const override;

        std::string getName() const override;

		std::string getDefaultFileExtension() const override;

    private:
        std::shared_ptr<::InferenceEngine::InferRequest> m_inferRequest;
		std::shared_ptr<::InferenceEngine::InferencePlugin> m_inferencePlugin;
        void loadPlugin(std::string deviceType);
};

}