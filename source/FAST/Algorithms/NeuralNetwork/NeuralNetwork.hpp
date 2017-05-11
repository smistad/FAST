#ifndef NEURAL_NETWORK_HPP_
#define NEURAL_NETWORK_HPP_

#include "FAST/ProcessObject.hpp"
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/framework/tensor.h>

namespace fast {

class Image;

class FAST_EXPORT  NeuralNetwork : public ProcessObject {
public:
    void load(std::string networkFilename);
    void setInputSize(int width, int height);
    void setOutputParameters(std::vector<std::string> outputNodeNames);
    void setScaleFactor(float scale);

    // Use this if only one output node
    tensorflow::Tensor getNetworkOutput();

    // Get output by layer name
    tensorflow::Tensor getNetworkOutput(std::string layerName);

    void loadAttributes();
protected:
    NeuralNetwork();
    UniquePointer<tensorflow::Session> mSession;
    bool mModelLoaded;
    bool mHasKerasLearningPhaseTensor;
    int mWidth;
    int mHeight;
    float mScaleFactor;
    std::string mInputName;
    std::vector<std::string> mOutputNames;
    std::map<std::string, tensorflow::Tensor> mOutputData;
    SharedPointer<Image> mImage;

    void execute();

    void executeNetwork(const std::vector<SharedPointer<Image> >& images);
    std::vector<SharedPointer<Image> > resizeImages(const std::vector<SharedPointer<Image> >& images);
};

}

#endif