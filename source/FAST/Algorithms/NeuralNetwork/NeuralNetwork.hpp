#ifndef NEURAL_NETWORK_HPP_
#define NEURAL_NETWORK_HPP_

#include "FAST/ProcessObject.hpp"
#include <tensorflow/core/public/session.h>

namespace fast {

class Image;

class NeuralNetwork : public ProcessObject {
public:
    void load(std::string networkFilename);
    void setInputSize(int width, int height);
    void setOutputParameters(std::vector<std::string> outputNodeNames);
    void setScaleFactor(float scale);

    // Use this if only one output node
    // output[i][j] will return output value j for input image i
    std::vector<std::vector<float> > getNetworkOutput();

    // Get output by layer name
    // output[i][j] will return output value j for input image i
    std::vector<std::vector<float> > getNetworkOutput(std::string layerName);

protected:
    NeuralNetwork();
    UniquePointer<tensorflow::Session> mSession;
    bool mModelLoaded;
    int mWidth;
    int mHeight;
    float mScaleFactor;
    std::string mInputName;
    std::vector<std::string> mOutputNames;
    std::map<std::string, std::vector<std::vector<float>> > mOutputData;

    void execute();

    void executeNetwork(const std::vector<SharedPointer<Image> >& images);
    std::vector<SharedPointer<Image> > resizeImages(const std::vector<SharedPointer<Image> >& images);
};

}

#endif