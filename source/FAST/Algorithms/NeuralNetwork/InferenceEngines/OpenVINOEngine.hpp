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

        std::vector<ModelFormat> getSupportedModelFormats() const {
            return { ModelFormat::OPENVINO, ModelFormat::ONNX };
        };

        ModelFormat getPreferredModelFormat() const {
            return ModelFormat::OPENVINO;
        };

        /**
         * Get a list of devices available for this inference engine.
         *
         * @return vector with info on each device
         */
        std::vector<InferenceDeviceInfo> getDeviceList() override;

        /**
         * Load a custom operator (op). You have to do this BEFORE calling load() to load the model/graph.
         *
         * @param filenames (can be a .so/.dll file for CPU plugin, or .xml for GPU/VPU plugins
         */
        void loadCustomPlugins(std::vector<std::string> filename) override;

        ~OpenVINOEngine();
    private:
        std::shared_ptr<::InferenceEngine::Core> m_inferenceCore;
        // This has to be last, because then inferRequest will be deleted before the plugin, which is necessary to avoid a crash on delete
        std::shared_ptr<::InferenceEngine::InferRequest> m_inferRequest;
		
        void loadPlugin(std::string deviceType);

        // This mutex is used to ensure only one thread is using this OpenVINO instance at the same time
        std::mutex m_mutex;
};

DEFINE_INFERENCE_ENGINE(OpenVINOEngine, INFERENCEENGINEOPENVINO_EXPORT)

}