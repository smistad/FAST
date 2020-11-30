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
        std::string getDefaultFileExtension() const override;
        TensorFlowEngine();
        /**
         * Get a list of devices available for this inference engine.
         *
         * @return vector with info on each device
         */
        std::vector<InferenceDeviceInfo> getDeviceList();
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
    FAST_OBJECT(TensorFlowTensor)
    public:
        void create(TensorFlowTensorWrapper* tensorflowTensor);
        ~TensorFlowTensor();
    private:
        TensorFlowTensorWrapper* m_tensorflowTensor;
        float* getHostDataPointer() override;
        bool hasAnyData() override;
};

}
