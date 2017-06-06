
#include "FAST/Exception.hpp"
#include "FileStreamer.hpp"
#include <fstream>
#include <chrono>
#include "FAST/Data/Image.hpp" // TODO should not be here

namespace fast {

FileStreamer::FileStreamer() {
    mStreamIsStarted = false;
    mNrOfReplays = 0;
    mIsModified = true;
    mLoop = false;
    mStartNumber = 0;
    mZeroFillDigits = 0;
    mFirstFrameIsInserted = false;
    mHasReachedEnd = false;
    mTimestampFilename = "";
    mNrOfFrames = 0;
    mSleepTime = 0;
    mStepSize = 1;
    mMaximumNrOfFramesSet = false;
    mStop = false;
}

void FileStreamer::setNumberOfReplays(uint replays) {
    mNrOfReplays = replays;
}

void FileStreamer::setStreamingMode(StreamingMode mode) {
    if(mode == STREAMING_MODE_STORE_ALL_FRAMES && !mMaximumNrOfFramesSet)
        setMaximumNumberOfFrames(0);
    Streamer::setStreamingMode(mode);
}

uint FileStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

void FileStreamer::setSleepTime(uint milliseconds) {
    mSleepTime = milliseconds;
}

void FileStreamer::setMaximumNumberOfFrames(uint nrOfFrames) {
    mMaximumNrOfFrames = nrOfFrames;
    DynamicData::pointer data = getDynamicOutputData();
    data->setMaximumNumberOfFrames(nrOfFrames);
}

void FileStreamer::setTimestampFilename(std::string filepath) {
    mTimestampFilename = filepath;
}

void FileStreamer::execute() {
    getDynamicOutputData()->setStreamer(mPtr.lock());
    if(mFilenameFormats.size() == 0)
        throw Exception("No filename format was given to the FileStreamer");
    if(!mStreamIsStarted) {
        // TODO Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        mThread = new std::thread(std::bind(&FileStreamer::producerStream, this));
    }

    // Wait here for first frame
    std::unique_lock<std::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}

void FileStreamer::setFilenameFormat(std::string str) {
    if(str.find("#") == std::string::npos)
        throw Exception("Filename format must include a hash tag # which will be replaced by a integer starting from 0.");

    mFilenameFormats.clear();
    mFilenameFormats.push_back(str);
}

void FileStreamer::setFilenameFormats(std::vector<std::string> strs) {
    for(std::string str : strs) {
        if(str.find("#") == std::string::npos)
            throw Exception("Filename format must include a hash tag # which will be replaced by a integer starting from 0.");
    }
    mFilenameFormats = strs;
}

void FileStreamer::producerStream() {
    Streamer::pointer pointerToSelf = mPtr.lock(); // try to avoid this object from being destroyed until this function is finished

    // Read timestamp file if available
    std::ifstream timestampFile;
    unsigned long previousTimestamp = 0;
    auto previousTimestampTime = std::chrono::high_resolution_clock::time_point::min();
    if(mTimestampFilename != "") {
        timestampFile.open(mTimestampFilename.c_str());
        if(!timestampFile.is_open()) {
            throw Exception("Timestamp file not found in FileStreamer");
        }

        // Fast forward to start
        if(mStartNumber > 0) {
            int i = 0;
            while(i <= mStartNumber) {
                std::string line;
                std::getline(timestampFile, line);
                ++i;
            }
        }
    }

    uint i = mStartNumber;
    int replays = 0;
    int currentSequence = 0;
    while(true) {
        {
            std::unique_lock<std::mutex> lock(mStopMutex);
            if(mStop) {
                mStreamIsStarted = false;
                mFirstFrameIsInserted = false;
                mHasReachedEnd = false;
                break;
            }
        }
        std::string filename = mFilenameFormats[currentSequence];
        std::string frameNumber = std::to_string(i);
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
            DataObject::pointer dataFrame = getDataFrame(filename);
            // Set and use timestamp if available
            if(mTimestampFilename != "") {
                std::string line;
                std::getline(timestampFile, line);
                if(line != "") {
                    unsigned long timestamp = std::stoul(line);
                    dataFrame->setCreationTimestamp(timestamp);
                    // Wait as long as necessary before adding image
                    auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - previousTimestampTime);
                    //reportInfo() << timestamp << reportEnd();
                    //reportInfo() << previousTimestamp << reportEnd();
                    //reportInfo() << "Time passed: " << timePassed.count() << reportEnd();
                    while(timestamp > previousTimestamp + timePassed.count()) {
                        // Wait
                        std::this_thread::sleep_for(std::chrono::milliseconds(timestamp-(long)previousTimestamp-timePassed.count()));
                        timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::high_resolution_clock::now() - previousTimestampTime);
                        //reportInfo() << "wait" << reportEnd();
                        //reportInfo() << timestamp << reportEnd();
                        //reportInfo() << previousTimestamp << reportEnd();
                        //reportInfo() << "Time passed: " << timePassed.count() << reportEnd();
                    }
                    previousTimestamp = timestamp;
                    previousTimestampTime = std::chrono::high_resolution_clock::now();
                }
            }
            DynamicData::pointer ptr = getDynamicOutputData();
            if(ptr.isValid()) {
                try {
                    ptr->addFrame(dataFrame);
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
                if(mSleepTime > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(mSleepTime));
            } else {
                reportInfo() << "DynamicImage object destroyed, stream can stop." << Reporter::end();
                break;
            }
            mNrOfFrames++;
            i += mStepSize;
        } catch(FileNotFoundException &e) {
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
                if(mLoop ||
                   (mNrOfReplays > 0 && replays != mNrOfReplays) ||
                   (currentSequence < mFilenameFormats.size()-1)) {
                    // Restart stream
                    if(timestampFile.is_open()) {
                        previousTimestamp = 0;
                        previousTimestampTime = std::chrono::high_resolution_clock::time_point::min();
                        timestampFile.seekg(0); // reset file to start
                    }
                    replays++;
                    i = mStartNumber;
                    currentSequence++;
                    // Go to first sequence if looping is enabled
                    if(mLoop && currentSequence == mFilenameFormats.size()) {
                        currentSequence = 0;
                    }
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

FileStreamer::~FileStreamer() {
    if(mStreamIsStarted) {
        if(mThread->get_id() != std::this_thread::get_id()) { // avoid deadlock
            stop();
            delete mThread;
            mThread = NULL;
        }
    }
}

bool FileStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

void FileStreamer::setStartNumber(uint startNumber) {
    mStartNumber = startNumber;
}

void FileStreamer::setZeroFilling(uint digits) {
    mZeroFillDigits = digits;
}

void FileStreamer::enableLooping() {
    mLoop = true;
}

void FileStreamer::disableLooping() {
    mLoop = false;
}

void FileStreamer::setStepSize(uint stepSize) {
    if(stepSize == 0)
        throw Exception("Step size given to FileStreamer can't be 0");
    mStepSize = stepSize;
}

void FileStreamer::stop() {
    {
        std::unique_lock<std::mutex> lock(mStopMutex);
        mStop = true;
    }
    mThread->join();
    std::cout << "File streamer thread returned" << std::endl;
}

} // end namespace fast
