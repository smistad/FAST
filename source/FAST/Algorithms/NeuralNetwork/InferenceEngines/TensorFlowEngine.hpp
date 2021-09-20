#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>
#include <TensorFlowExport.hpp>

// Forward declare
namespace tensorflow {
class Session;
class SavedModelBundle;
}

namespace fast {

class TensorFlowEngine : public InferenceEngine {
    public:
        void load() override;
        void run() override;
        std::string getName() const override;
        ~TensorFlowEngine() override;
        ImageOrdering getPreferredImageOrdering() const override;
		std::vector<ModelFormat> getSupportedModelFormats() const {
            return { ModelFormat::PROTOBUF, ModelFormat::SAVEDMODEL };
        };

        ModelFormat getPreferredModelFormat() const {
            return ModelFormat::PROTOBUF;
        };

        TensorFlowEngine();
        /**
         * Get a list of devices available for this inference engine.
         *
         * @return vector with info on each device
         */
        std::vector<InferenceDeviceInfo> getDeviceList();
        /**
         * Load a custom operator (op). You have to do this BEFORE calling load() to load the model/graph.
         *
         * @param filename
         */
        void loadCustomPlugins(std::vector<std::string> filename) override;
    protected:
        std::unique_ptr<tensorflow::Session> mSession;
        std::unique_ptr<tensorflow::SavedModelBundle> mSavedModelBundle;
        std::vector<std::string> mLearningPhaseTensors;

};

DEFINE_INFERENCE_ENGINE(TensorFlowEngine, INFERENCEENGINETENSORFLOW_EXPORT)


// Forward declare
class TensorFlowTensorWrapper;

/**
 * This specialized Tensor Data class, allow us to store Tensorflow type tensors as FAST tensors.
 */
class TensorFlowTensor : public Tensor {
    FAST_DATA_OBJECT(Tensor)
    public:
        FAST_CONSTRUCTOR(TensorFlowTensor, TensorFlowTensorWrapper*, tensorflowTensor,);
        ~TensorFlowTensor();
    private:
        TensorFlowTensorWrapper* m_tensorflowTensor;
        float* getHostDataPointer() override;
        bool hasAnyData() override;
};

}
