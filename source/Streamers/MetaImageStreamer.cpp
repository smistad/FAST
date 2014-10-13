#include "MetaImageStreamer.hpp"
#include "MetaImageImporter.hpp"
#include "DeviceManager.hpp"
#include "Exception.hpp"
#include <boost/lexical_cast.hpp>

namespace fast {
/**
 * Dummy function to get into the class again
 */
inline void stubStreamThread(MetaImageStreamer * streamer) {
    streamer->producerStream();
}

DynamicImage::pointer MetaImageStreamer::getOutput() {
    mOutput->setSource(mPtr.lock());
    mOutput->setStreamer(mPtr.lock());
    return mOutput;
}

MetaImageStreamer::MetaImageStreamer() {
    mOutput = DynamicImage::New();
    mStreamIsStarted = false;
    mIsModified = true;
    mLoop = false;
    mStartNumber = 0;
    mZeroFillDigits = 0;
    thread = NULL;
    mFirstFrameIsInserted = false;
    mHasReachedEnd = false;
    mFilenameFormat = "";
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
}


void MetaImageStreamer::execute() {
    mOutput->setSource(mPtr.lock());
    mOutput->setStreamer(mPtr.lock());
    if(mFilenameFormat == "")
        throw Exception("No filename format was given to the MetaImageStreamer");
    if(!mStreamIsStarted) {
        mStreamIsStarted = true;
        thread = new boost::thread(&stubStreamThread, this);
    }

    // Wait here for first frame
    boost::unique_lock<boost::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}

void MetaImageStreamer::setFilenameFormat(std::string str) {
    if(str.find("#") == std::string::npos)
        throw Exception("Filename format must include a hash tag # which will be replaced by a integer starting from 0.");
    mFilenameFormat = str;
}

void MetaImageStreamer::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
}


void MetaImageStreamer::producerStream() {
    uint i = mStartNumber;
    while(true) {
        std::string filename = mFilenameFormat;
        std::string frameNumber = boost::lexical_cast<std::string>(i);
        if(mZeroFillDigits > 0 && frameNumber.size() < mZeroFillDigits) {
            std::string zeroFilling = "";
            for(uint z = 0; z < mZeroFillDigits-frameNumber.size(); z++) {
                zeroFilling += "0";
            }
            frameNumber = zeroFilling + frameNumber;
        }
        filename.replace(
                filename.find("#"),
                1,
                frameNumber
                );
        try {
            MetaImageImporter::pointer importer = MetaImageImporter::New();
            importer->setFilename(filename);
            importer->setDevice(mDevice);
            Image::pointer image = importer->getOutput();
            image->update();
            DynamicImage::pointer ptr = mOutput;
            if(ptr.isValid()) {
                try {
                    ptr->addFrame(image);
                } catch(Exception &e) {
                    std::cout << "streamer has been deleted, stop" << std::endl;
                    break;
                }
                {
                    boost::lock_guard<boost::mutex> lock(mFirstFrameMutex);
                    mFirstFrameIsInserted = true;
                }
                mFirstFrameCondition.notify_one();
            } else {
                std::cout << "DynamicImage object destroyed, stream can stop." << std::endl;
                break;
            }
            i++;
        } catch(FileNotFoundException &e) {
            if(i > 0) {
                std::cout << "Reached end of stream" << std::endl;
                if(mLoop) {
                    // Restart stream
                    i = mStartNumber;
                    continue;
                }
                mHasReachedEnd = true;
                // Reached end of stream
                break;
            } else {
                throw e;
            }
        }
    }
}

MetaImageStreamer::~MetaImageStreamer() {
    if(mStreamIsStarted) {
        std::cout << "Joining the thread" << std::endl;
        // TODO stop thread as well
        thread->join();
        delete thread;
    }
}

bool MetaImageStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

void MetaImageStreamer::setStartNumber(uint startNumber) {
    mStartNumber = startNumber;
}

void MetaImageStreamer::setZeroFilling(uint digits) {
    mZeroFillDigits = digits;
}

void MetaImageStreamer::enableLooping() {
    mLoop = true;
}

void MetaImageStreamer::disableLooping() {
    mLoop = false;
}

} // end namespace fast
