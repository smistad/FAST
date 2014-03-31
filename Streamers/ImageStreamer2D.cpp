#include "ImageStreamer2D.hpp"
#include "ImageImporter2D.hpp"
#include "DeviceManager.hpp"
#include "Exception.hpp"
using namespace fast;

/**
 * Dummy function to get into the class again
 */
void stubStreamThread(ImageStreamer2D * streamer) {
    streamer->producerStream();
}

Image2Dt::pointer ImageStreamer2D::getOutput() {
    if(!mStreamIsStarted) {
        mStreamIsStarted = true;
        thread = new boost::thread(&stubStreamThread, this);
    }

    // Wait here for first frame
    // TODO use condition variable instead
    while(!mFirstFrameIsInserted);

    if(mOutput.isValid()) {
        mOutput->setParent(mPtr.lock());

        Image2Dt::pointer newSmartPtr;
        newSmartPtr.swap(mOutput);

        return newSmartPtr;
    } else {
        return mOutput2.lock();
    }
}

ImageStreamer2D::ImageStreamer2D() {
    mOutput = Image2Dt::New();
    mOutput2 = mOutput;
    mStreamIsStarted = false;
    mIsModified = true;
    thread = NULL;
    mFirstFrameIsInserted = false;
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
}


inline void ImageStreamer2D::execute() {
    if(!mStreamIsStarted) {
        mStreamIsStarted = true;
        thread = new boost::thread(&stubStreamThread, this);
    }

    // Wait here for first frame
    // TODO use condition variable instead
    while(!mFirstFrameIsInserted);
}

void ImageStreamer2D::setFilenameFormat(std::string str) {
    mFilenameFormat = str;
}

void ImageStreamer2D::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
}

inline std::string intToString(int number) {
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

void ImageStreamer2D::producerStream() {
    int i = 0;
    while(true) {
        std::string filename = mFilenameFormat;
        filename.replace(
                filename.find("#"),
                1,
                intToString(i));
        std::cout << filename << std::endl;
        try {
            ImageImporter2D::pointer importer = ImageImporter2D::New();
            importer->setFilename(filename);
            importer->setDevice(mDevice);
            Image2D::pointer image = importer->getOutput();
            image->update();
            Image2Dt::pointer ptr = mOutput2.lock();
            if(ptr.isValid()) {
                ptr->addFrame(image);
                mFirstFrameIsInserted = true;
            } else {
                std::cout << "Image2Dt object destroyed, stream can stop." << std::endl;
                break;
            }
            i++;
        } catch(FileNotFoundException &e) {
            std::cout << "Reached end of stream" << std::endl;
            // Reached end of stream
            break;
        }
    }
}

ImageStreamer2D::~ImageStreamer2D() {
    std::cout << "Joining the thread" << std::endl;
    // TODO stop thread as well
    thread->join();
    delete thread;
}
