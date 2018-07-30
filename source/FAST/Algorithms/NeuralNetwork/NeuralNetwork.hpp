#ifndef NEURAL_NETWORK_HPP_
#define NEURAL_NETWORK_HPP_

#include "FAST/ProcessObject.hpp"
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/framework/tensor.h>
#include <queue>

namespace fast {

class Image;

class FAST_EXPORT  NeuralNetwork : public ProcessObject {
    FAST_OBJECT(NeuralNetwork)
public:
    void load(std::string networkFilename);
    void setInputSize(int width, int height);
    void setInputName(std::string inputName);
    void setOutputParameters(std::vector<std::string> outputNodeNames);
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
     * Set the temporal window.
     * If window > 1, assume the input tensor has 5 dimensions (batch_size, frames, height, width, channels)
     * If the window is set to 4, the frames t-3, t-2, t-1 and t, where t is the current timestep,
     * will be given as input to the network.
     *
     * @param window
     */
    void setTemporalWindow(uint window);

    void addTemporalImageFrame(SharedPointer<Image> image);

    // Use this if only one output node
    tensorflow::Tensor getNetworkOutput();

    // Get output by layer name
    tensorflow::Tensor getNetworkOutput(std::string layerName);

    void loadAttributes();

    virtual ~NeuralNetwork();
protected:
    NeuralNetwork();
    std::unique_ptr<tensorflow::Session> mSession;
    bool mModelLoaded;
    bool mPreserveAspectRatio;
    bool mHorizontalImageFlipping = false;
    bool mSignedInputNormalization = false;
    std::vector<std::string> mLearningPhaseTensors;
    std::vector<int> mInputShape;
    uint mTemporalWindow = 1;
    float mScaleFactor;
    std::string mInputName;
    std::vector<std::string> mOutputNames;
    std::map<std::string, tensorflow::Tensor> mOutputData;
    std::deque<SharedPointer<Image>> mImages;
    Vector3f mNewInputSpacing;



    void execute();

    void executeNetwork(const std::vector<SharedPointer<Image> >& images);
    std::vector<SharedPointer<Image> > resizeImages(const std::vector<SharedPointer<Image> >& images);
};

}

#endif