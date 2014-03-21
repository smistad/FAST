#include "ImageStreamer2D.hpp"
#include "ImageImporter2D.hpp"
#include "DeviceManager.hpp"
#include "Exception.hpp"
using namespace fast;

Image2Dt::Ptr ImageStreamer2D::getOutput() {
    return mOutput;
}

ImageStreamer2D::ImageStreamer2D() {
    mOutput = Image2Dt::New();
    mStreamIsStarted = false;
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
}
/**
 * Dummy function to get into the class again
 */
void stubStreamThread(ImageStreamer2D * streamer) {
    streamer->producerStream();
}
void ImageStreamer2D::execute() {
    if(!mStreamIsStarted) {
        mStreamIsStarted = true;
        thread = boost::thread(&stubStreamThread, this);
    }
}

void ImageStreamer2D::setFilenameFormat(std::string str) {
    mFilenameFormat = str;
}

void ImageStreamer2D::setDevice(ExecutionDevice::Ptr device) {
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
        std::string filename = mFilenameFormat.replace(mFilenameFormat.find("#"), 1, intToString(i));
        try {
            ImageImporter2D::Ptr importer = ImageImporter2D::New();
            importer->setFilename(filename);
            importer->setDevice(mDevice);
            importer->update();
            mOutput->addFrame(importer->getOutput());
            i++;
        } catch(FileNotFoundException &e) {
            // Reached end of stream
            break;
        }
    }
}
