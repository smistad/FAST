#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"

#include "ManualImageStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <fstream>
#include <chrono>

namespace fast {

ManualImageStreamer::ManualImageStreamer() {
    mNrOfReplays = 0;
    mIsModified = true;
    mLoop = false;
    mStartNumber = 0;
    mZeroFillDigits = 0;
    mNrOfFrames = 0;
    mSleepTime = 0;
    mStepSize = 1;
    mMaximumNrOfFramesSet = false;
    createOutputPort<Image>(0);
}

void ManualImageStreamer::addImage(Image::pointer image) {
    if(mImages.empty())
        mImages.push_back(std::vector<Image::pointer>());
    mImages[0].push_back(image);
}

void ManualImageStreamer::addSequence(std::vector<std::shared_ptr<Image>> images) {
    mImages.push_back(images);
}

void ManualImageStreamer::setNumberOfReplays(uint replays) {
	mNrOfReplays = replays;
}

uint ManualImageStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

void ManualImageStreamer::setSleepTime(uint milliseconds) {
    mSleepTime = milliseconds;
}

void ManualImageStreamer::execute() {
    if(mImages.size() == 0)
    	throw Exception("No images added to the manual image streamer.");
    startStream();
    waitForFirstFrame();
}


void ManualImageStreamer::generateStream() {
    uint i = mStartNumber;
    int replays = 0;
    uint64_t previousTimestamp = 0;
    auto previousTimestampTime = std::chrono::high_resolution_clock::time_point::min();
    int currentSequence = 0;
    while(true) {
        {
            std::unique_lock<std::mutex> lock(m_stopMutex);
            if(m_stop) {
                m_streamIsStarted = false;
                m_firstFrameIsInserted = false;
                break;
            }
        }
        try {
            auto image = mImages[currentSequence].at(i);
            addOutputData(0, image);
            if(image->getCreationTimestamp() != 0) {
                uint64_t timestamp = image->getCreationTimestamp();
                // Wait as long as necessary before adding image
                // Time passed since last frame
                auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - previousTimestampTime);
                while (timestamp > previousTimestamp + timePassed.count()) {
                    // Wait
                    int64_t left = (timestamp - previousTimestamp) - timePassed.count();
                    reportInfo() << "Sleeping for " << left << " ms" << reportEnd();
                    std::this_thread::sleep_for(std::chrono::milliseconds(left));
                    timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - previousTimestampTime);
                }
                previousTimestamp = timestamp;
                previousTimestampTime = std::chrono::high_resolution_clock::now();
            }
            if(mSleepTime > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTime));
            frameAdded();
            mNrOfFrames++;
            i += mStepSize;
        } catch(std::out_of_range &e) {
            if(i > 0) {
                reportInfo() << "Reached end of stream" << Reporter::end();
                // If there where no files found at all, we need to release the execute method
                frameAdded();
                if(mLoop || (mNrOfReplays > 0 && replays != mNrOfReplays || (currentSequence < mImages.size()-1))) {
                    // Restart stream
                    previousTimestamp = 0;
                    previousTimestampTime = std::chrono::high_resolution_clock::time_point::min();
                    replays++;
                    i = mStartNumber;
                    currentSequence++;
                    // Go to first sequence if looping is enabled
                    if(mLoop && currentSequence == mImages.size()) {
                        currentSequence = 0;
                    }
                    continue;
                }
                // Reached end of stream
                break;
            } else {
                throw e;
            }
        }
    }
}

ManualImageStreamer::~ManualImageStreamer() {
    stop();
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
