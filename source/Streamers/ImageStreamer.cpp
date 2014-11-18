#include "ImageStreamer.hpp"
#include "ImageImporter.hpp"
#include "DeviceManager.hpp"
#include "Exception.hpp"
using namespace fast;

/**
 * Dummy function to get into the class again
 */
inline void stubStreamThread(ImageStreamer * streamer) {
    streamer->producerStream();
}

DynamicImage::pointer ImageStreamer::getOutput() {
    mOutput->setSource(mPtr.lock());
    mOutput->setStreamer(mPtr.lock());
    return mOutput;
}

ImageStreamer::ImageStreamer() {
    mOutput = DynamicImage::New();
    mStreamIsStarted = false;
    mIsModified = true;
    thread = NULL;
    mFirstFrameIsInserted = false;
    mHasReachedEnd = false;
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
    mFilenameFormat = "";
}


void ImageStreamer::execute() {
    mOutput->setSource(mPtr.lock());
    mOutput->setStreamer(mPtr.lock());
    if(mFilenameFormat == "")
        throw Exception("No filename format was given to the ImageStreamer");
    if(!mStreamIsStarted) {
        mStreamIsStarted = true;
        thread = new boost::thread(&stubStreamThread, this);
    }

    // Wait here for first frame
    // TODO use condition variable instead
    while(!mFirstFrameIsInserted);
}

void ImageStreamer::setFilenameFormat(std::string str) {
    if(str.find("#") == std::string::npos)
        throw Exception("Filename format must include a hash tag # which will be replaced by a integer starting from 0.");
    mFilenameFormat = str;
}

void ImageStreamer::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
}

inline std::string intToString(int number) {
    std::stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

void ImageStreamer::producerStream() {
    int i = 0;
    while(true) {
        std::string filename = mFilenameFormat;
        filename.replace(
                filename.find("#"),
                1,
                intToString(i));
        try {
            ImageImporter::pointer importer = ImageImporter::New();
            importer->setFilename(filename);
            importer->setDevice(mDevice);
            Image::pointer image = importer->getOutput();
            image->update();
            DynamicImage::pointer ptr = mOutput;
            if(ptr.isValid()) {
                ptr->addFrame(image);
                mFirstFrameIsInserted = true;
            } else {
                std::cout << "DynamicImage object destroyed, stream can stop." << std::endl;
                break;
            }
            i++;
        } catch(FileNotFoundException &e) {
            std::cout << "Reached end of stream" << std::endl;
            mHasReachedEnd = true;
            // Reached end of stream
            break;
        }
    }
}

ImageStreamer::~ImageStreamer() {
    if(mStreamIsStarted) {
        if(thread->get_id() != boost::this_thread::get_id()) { // avoid deadlock
            thread->join();
        }
        delete thread;
    }
}
bool ImageStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}
