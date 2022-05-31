#include "ImageToBatchGenerator.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>

namespace fast {

ImageToBatchGenerator::ImageToBatchGenerator() {
    createInputPort<Image>(0, false);
    createOutputPort<Batch>(0);

    m_maxBatchSize = -1;
    mIsModified = true;

    createIntegerAttribute("max-batch-size", "Maximum batch size", "", m_maxBatchSize);
}

ImageToBatchGenerator::ImageToBatchGenerator(int maxBatchSize) : ImageToBatchGenerator() {
    setMaxBatchSize(maxBatchSize);
}

void ImageToBatchGenerator::loadAttributes() {
    setMaxBatchSize(getIntegerAttribute("max-batch-size"));
}

void ImageToBatchGenerator::generateStream() {
    std::vector<Image::pointer> imageList;
    imageList.reserve(m_maxBatchSize);
    int i = 0;
    bool lastFrame = false;
    // Update will eventually block, therefore we need to call this in a separate thread
    auto po = mParent->getProcessObject();
    bool firstTime = true;
    while(!lastFrame) {
        {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop) {
                m_streamIsStarted = false;
                m_firstFrameIsInserted = false;
                break;
            }
        }
        if(!firstTime) // parent is execute the first time, thus drop it here
            po->update(); // Make sure execute is called on previous
        firstTime = false;
        Image::pointer image;
        try {
            image = mParent->getNextFrame<Image>();
        } catch(ThreadStopped &e) {
            break;
        }
        lastFrame = image->isLastFrame();
        imageList.push_back(image);
        if(imageList.size() == m_maxBatchSize || lastFrame) {
            auto batch = Batch::create(imageList);
            if(lastFrame)
                batch->setLastFrame(getNameOfClass());
            try {
                addOutputData(0, batch);
            } catch(ThreadStopped &e) {
                break;
            }
            frameAdded();
            imageList.clear();
            i++;
        }
    }
    //updateThread.join();
}

void ImageToBatchGenerator::execute() {
    if(m_maxBatchSize == -1)
        throw Exception("Max batch size must be given to the ImageToBatchGenerator");

    if(!m_streamIsStarted) {
        m_streamIsStarted = true;
        mParent = mInputConnections[0];
        mInputConnections.clear();
        m_thread = std::make_unique<std::thread>(std::bind(&ImageToBatchGenerator::generateStream, this));
    }

    waitForFirstFrame();
}

void ImageToBatchGenerator::setMaxBatchSize(int size) {
    if(size <= 0)
        throw Exception("Max batch size must be larger than 0");
    m_maxBatchSize = size;
    mIsModified = true;
}

ImageToBatchGenerator::~ImageToBatchGenerator() {
    stop();
}

}