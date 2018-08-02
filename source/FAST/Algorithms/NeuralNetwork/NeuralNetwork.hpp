#ifndef NEURAL_NETWORK_HPP_
#define NEURAL_NETWORK_HPP_

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/SimpleDataObject.hpp>

// Forward declare
namespace tensorflow {
class Session;
}

namespace fast {

class Image;
class Tensor;

FAST_SIMPLE_DATA_OBJECT(Batch, std::vector<SharedPointer<Image>>)

FAST_SIMPLE_DATA_OBJECT(Sequence, std::vector<SharedPointer<Image>>)

class FAST_EXPORT NeuralNetwork : public ProcessObject {
    FAST_OBJECT(NeuralNetwork)
    public:
    enum class NodeType {
        IMAGE,
        TENSOR,
    };
    void load(std::string networkFilename);
    void addInputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
    void addOutputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
    void setScaleFactor(float scale);
    void setSignedInputNormalization(bool signedInputNormalization);
    void setPreserveAspectRatio(bool preserve);
    /**
     * Setting this parameter to true will flip the input image horizontally.
     * For pixel classification the output image will be flipped back.
     * @param flip
     */
    void setHorizontalFlipping(bool flip);

    /**
     * Set the temporal window for dynamic mode.
     * If window > 1, assume the second dimension of the input tensor is the number of timesteps.
     * If the window is set to 4, the frames t-3, t-2, t-1 and t, where t is the current timestep,
     * will be given as input to the network.
     *
     * @param window
     */
    void setTemporalWindow(uint window);

    void loadAttributes();

    virtual ~NeuralNetwork();
protected:
    NeuralNetwork();
    std::unique_ptr<tensorflow::Session> mSession;
    bool mModelLoaded;
    bool mPreserveAspectRatio;
    bool mHorizontalImageFlipping = false;
    bool mSignedInputNormalization = false;
    int mTemporalWindow = 0;
    std::vector<std::string> mLearningPhaseTensors;
    float mScaleFactor;
    Vector3f mNewInputSpacing;

    struct NetworkNode {
        uint portID;
        NodeType type;
        TensorShape shape;
    };

    std::unordered_map<std::string, NetworkNode> mInputNodes;
    std::unordered_map<std::string, NetworkNode> mOutputNodes;
    std::unordered_map<std::string, std::vector<SharedPointer<Image>>> mInputImages;

    std::unordered_map<std::string, Tensor::pointer> processInputData();
    std::vector<std::pair<NetworkNode, SharedPointer<Tensor>>> executeNetwork(std::unordered_map<std::string, Tensor::pointer> tensors);
    std::vector<SharedPointer<Image>> resizeImages(const std::vector<SharedPointer<Image>>& images, int width, int height, int depth);
    Tensor::pointer convertImagesToTensor(std::vector<SharedPointer<Image>> image, const TensorShape& shape, bool temporal);

    private:
        void execute();
};

}

#endif