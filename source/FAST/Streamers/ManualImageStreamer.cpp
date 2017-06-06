#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"

#include "ManualImageStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <fstream>
#include <chrono>

namespace fast {
/**
 * Dummy function to get into the class again
 */
inline void stubStreamThread(ManualImageStreamer * streamer) {
    streamer->producerStream();
}

ManualImageStreamer::ManualImageStreamer() {
    mStreamIsStarted = false;
    mNrOfReplays = 0;
    mIsModified = true;
    mLoop = false;
    mStartNumber = 0;
    mZeroFillDigits = 0;
    thread = NULL;
    mFirstFrameIsInserted = false;
    mHasReachedEnd = false;
    mNrOfFrames = 0;
    mSleepTime = 0;
    mStepSize = 1;
    mMaximumNrOfFramesSet = false;
    createOutputPort<Image>(0, OUTPUT_DYNAMIC);
    setMaximumNumberOfFrames(50); // Set default maximum number of frames to 50
}

void ManualImageStreamer::addImage(Image::pointer image) {
	mImages.push_back(image);
}

void ManualImageStreamer::setNumberOfReplays(uint replays) {
	mNrOfReplays = replays;
}

void ManualImageStreamer::setStreamingMode(StreamingMode mode) {
    if(mode == STREAMING_MODE_STORE_ALL_FRAMES && !mMaximumNrOfFramesSet)
        setMaximumNumberOfFrames(0);
    Streamer::setStreamingMode(mode);
}

uint ManualImageStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

void ManualImageStreamer::setSleepTime(uint milliseconds) {
    mSleepTime = milliseconds;
}

void ManualImageStreamer::setMaximumNumberOfFrames(uint nrOfFrames) {
    mMaximumNrOfFrames = nrOfFrames;
    DynamicData::pointer data = getOutputData<Image>(0);
    data->setMaximumNumberOfFrames(nrOfFrames);
}

void ManualImageStreamer::execute() {
    getOutputData<Image>(0)->setStreamer(mPtr.lock());
    if(mImages.size() == 0)
    	throw Exception("No images added to the manual image streamer.");
    if(!mStreamIsStarted) {
        // Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        thread = new std::thread(&stubStreamThread, this);
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}


void ManualImageStreamer::producerStream() {
    Streamer::pointer pointerToSelf = mPtr.lock(); // try to avoid this object from being destroyed until this function is finished

    uint i = mStartNumber;
    int replays = 0;
    while(true) {
        try {
            Image::pointer image = mImages.at(i);
            DynamicData::pointer ptr = getOutputData<Image>();
            if(ptr.isValid()) {
                try {
                    ptr->addFrame(image);
                    if(mSleepTime > 0)
                        std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTime));
                } catch(NoMoreFramesException &e) {
                    throw e;
                } catch(Exception &e) {
                    reportInfo() << "streamer has been deleted, stop" << Reporter::end();
                    break;
                }
                if(!mFirstFrameIsInserted) {
                    {
                        std::lock_guard<std::mutex> lock(mFirstFrameMutex);
                        mFirstFrameIsInserted = true;
                    }
                    mFirstFrameCondition.notify_one();
                }
            } else {
                reportInfo() << "DynamicImage object destroyed, stream can stop." << Reporter::end();
                break;
            }
            mNrOfFrames++;
            i += mStepSize;
        } catch(std::out_of_range &e) {
            if(i > 0) {
                reportInfo() << "Reached end of stream" << Reporter::end();
                // If there where no files found at all, we need to release the execute method
                if(!mFirstFrameIsInserted) {
                    {
                        std::lock_guard<std::mutex> lock(mFirstFrameMutex);
                        mFirstFrameIsInserted = true;
                    }
                    mFirstFrameCondition.notify_one();
                }
                if(mLoop || (mNrOfReplays > 0 && replays != mNrOfReplays)) {
                    // Restart stream
                    replays++;
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

ManualImageStreamer::~ManualImageStreamer() {
    if(mStreamIsStarted) {
        if(thread->get_id() != std::this_thread::get_id()) { // avoid deadlock
            thread->join();
        }
        delete thread;
    }
}

bool ManualImageStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

void ManualImageStreamer::setStartNumber(uint startNumber) {
    mStartNumber = startNumber;
}

void ManualImageStreamer::setZeroFilling(uint digits) {
    mZeroFillDigits = digits;
}

void ManualImageStreamer::enableLooping() {
    mLoop = true;
}

void ManualImageStreamer::disableLooping() {
    mLoop = false;
}

void ManualImageStreamer::setStepSize(uint stepSize) {
    if(stepSize == 0)
        throw Exception("Step size given to ManualImageStreamer can't be 0");
    mStepSize = stepSize;
}

} // end namespace fast
