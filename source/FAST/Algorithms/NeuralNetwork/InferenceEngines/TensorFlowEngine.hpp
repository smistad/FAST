#pragma once

#include <FAST/Algorithms/NeuralNetwork/InferenceEngine.hpp>

// Forward declare
namespace tensorflow {
class Session;
class SavedModelBundle;
}

namespace fast {

class TensorFlowEngine : public InferenceEngine {
    public:
        virtual void load() override;
        virtual void run() override;
        virtual std::string getName() const = 0;
        ~TensorFlowEngine() override;
        virtual ImageOrdering getPreferredImageOrdering() const override;
        virtual std::string getDefaultFileExtension() const override;
        TensorFlowEngine();
    protected:
        std::unique_ptr<tensorflow::Session> mSession;
        std::unique_ptr<tensorflow::SavedModelBundle> mSavedModelBundle;
        std::vector<std::string> mLearningPhaseTensors;

};


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
