#ifndef NEURAL_NETWORK_HPP_
#define NEURAL_NETWORK_HPP_

#include <FAST/ProcessObject.hpp>
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/SimpleDataObject.hpp>
#include "InferenceEngine.hpp"



namespace fast {

class Image;
class Tensor;

FAST_SIMPLE_DATA_OBJECT(Batch, std::vector<SharedPointer<Image>>)

FAST_SIMPLE_DATA_OBJECT(Sequence, std::vector<SharedPointer<Image>>)

class FAST_EXPORT NeuralNetwork : public ProcessObject {
    FAST_OBJECT(NeuralNetwork)
    public:
        /**
         * Load a given netowrk file. This takes time.
         * @param filename
         */
        void load(std::string filename);
        /**
         * Specify which inference engine to use
         * @param engine
         */
        void setInferenceEngine(InferenceEngine::pointer engine);
        /**
         * Specify which inference engine to use
         * @param engine
         */
        void setInferenceEngine(std::string engine);
        /**
         * Retrieve current inference engine
         * @return
         */
        InferenceEngine::pointer getInferenceEngine() const;
    void addInputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
    void addOutputNode(uint portID, std::string name, NodeType type = NodeType::IMAGE, TensorShape shape = {});
    /**
     * For each input value i: new_i = i*scale
     * @param scale
     */
    void setScaleFactor(float scale);
    /**
     * For each input value i: new_i = (i - mean)/std, this is applied after the scale factor
     * @param mean
     * @param std
     */
    void setMeanAndStandardDeviation(float mean, float std);
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
    bool mPreserveAspectRatio;
    bool mHorizontalImageFlipping = false;
    bool mSignedInputNormalization = false;
    int mTemporalWindow = 0;
    std::vector<std::string> mLearningPhaseTensors;
    float mScaleFactor, mMean, mStd;
    Vector3f mNewInputSpacing;

    virtual void run();

    SharedPointer<InferenceEngine> m_engine;

    std::unordered_map<std::string, std::vector<SharedPointer<Image>>> mInputImages;

    std::unordered_map<std::string, Tensor::pointer> processInputData();
    std::vector<SharedPointer<Image>> resizeImages(const std::vector<SharedPointer<Image>>& images, int width, int height, int depth);
    Tensor::pointer convertImagesToTensor(std::vector<SharedPointer<Image>> image, const TensorShape& shape, bool temporal);

    private:
        void execute();
};

}

#endif