#ifndef NEURAL_NETWORK_HPP_
#define NEURAL_NETWORK_HPP_

#include "FAST/ProcessObject.hpp"
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/framework/tensor.h>
#include <queue>

namespace fast {

class Image;

class FAST_EXPORT  NeuralNetwork : public ProcessObject {
public:
    void load(std::string networkFilename);
    void setInputSize(int width, int height);
    void setInputName(std::string inputName);
    void setOutputParameters(std::vector<std::string> outputNodeNames);
    void setScaleFactor(float scale);
    void setPreserveAspectRatio(bool preserve);
    /**
     * Setting this parameter to true will flip the input image horizontally.
     * For pixel classification the output image will be flipped back.
     * @param flip
     */
    void setHorizontalFlipping(bool flip);

    /**
     * Set the nr of frames to keep and give to the network.
     * If this count is set to 4, the frames t-3, t-2, t-1 and t, where t is the current timestep,
     * will be given as input to the network. This can be useful for recurrent networks.
     * @param nrOfFrames
     */
    void setRememberFrames(uint nrOfFrames);

    // Use this if only one output node
    tensorflow::Tensor getNetworkOutput();

    // Get output by layer name
    tensorflow::Tensor getNetworkOutput(std::string layerName);

    void loadAttributes();
protected:
    NeuralNetwork();
    UniquePointer<tensorflow::Session> mSession;
    bool mModelLoaded;
    bool mPreserveAspectRatio;
    bool mHorizontalImageFlipping = false;
    std::vector<std::string> mLearningPhaseTensors;
    int mWidth;
    int mHeight;
    uint mFramesToRemember = 1;
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