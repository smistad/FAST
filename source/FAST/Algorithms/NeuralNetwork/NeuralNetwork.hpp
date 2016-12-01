#ifndef NEURAL_NETWORK_HPP_
#define NEURAL_NETWORK_HPP_

#include "FAST/ProcessObject.hpp"
#include <tensorflow/core/public/session.h>

namespace fast {

class Image;

class NeuralNetwork : public ProcessObject {
public:
    void load(std::string networkFilename);
    /**
     * This method is for loading a binary proto mean file
     * @param filename
     */
    void loadBinaryMeanImage(std::string filename);

    // Use this if only 1 network output layer
    std::vector<float> getNetworkOutput();
    // Get output by layer name
    std::vector<float> getNetworkOutput(std::string layerName);

    // TYPES OF DATA SIZE ADOPTION
    // Fit image to width and height (default), SQUASH
    // Keep aspect ratio and entire width: if height to high, cut on bottom, if height too low, add
protected:
    NeuralNetwork();
    UniquePointer<tensorflow::Session> mSession;
    bool mModelLoaded;
    cl::Image2D mMeanImage;
    bool mMeanImageLoaded;

    void execute();

    void executeNetwork(const std::vector<SharedPointer<Image> >& images);
    std::vector<SharedPointer<Image> > resizeImages(const std::vector<SharedPointer<Image> >& images);
    std::vector<SharedPointer<Image> > subtractMeanImage(const std::vector<SharedPointer<Image> >& images);
};

}

#endif