#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>
#include <OpenVINOExport.hpp>

namespace InferenceEngine {
class InferRequest;
class Core;
}

namespace fast {

class INFERENCEENGINEOPENVINO_EXPORT OpenVINOEngine : public InferenceEngine {
    FAST_OBJECT(OpenVINOEngine)
    public:
        void run() override;

        void load() override;

        ImageOrdering getPreferredImageOrdering() const override;

        std::string getName() const override;

		std::string getDefaultFileExtension() const override;

        ~OpenVINOEngine();
    private:
        std::shared_ptr<::InferenceEngine::Core> m_inferenceCore;
        // This has to be last, because then inferRequest will be deleted before the plugin, which is necessary to avoid a crash on delete
        std::shared_ptr<::InferenceEngine::InferRequest> m_inferRequest;
		
        void loadPlugin(std::string deviceType);
};

DEFINE_INFERENCE_ENGINE(OpenVINOEngine, INFERENCEENGINEOPENVINO_EXPORT)

}