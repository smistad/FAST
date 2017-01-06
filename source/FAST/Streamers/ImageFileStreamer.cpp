#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Exception.hpp"
#include <boost/lexical_cast.hpp>
#include "ImageFileStreamer.hpp"
#include "FAST/Data/Image.hpp"
#include <fstream>
#include <chrono>

namespace fast {
/**
 * Dummy function to get into the class again
 */
inline void stubStreamThread(ImageFileStreamer * streamer) {
    streamer->producerStream();
}

ImageFileStreamer::ImageFileStreamer() {
    mStreamIsStarted = false;
    mNrOfReplays = 0;
    mIsModified = true;
    mLoop = false;
    mStartNumber = 0;
    mZeroFillDigits = 0;
    thread = NULL;
    mFirstFrameIsInserted = false;
    mHasReachedEnd = false;
    mTimestampFilename = "";
    mNrOfFrames = 0;
    mSleepTime = 0;
    mStepSize = 1;
    mMaximumNrOfFramesSet = false;
    createOutputPort<Image>(0, OUTPUT_DYNAMIC);
    setMaximumNumberOfFrames(50); // Set default maximum number of frames to 50
}

void ImageFileStreamer::setNumberOfReplays(uint replays) {
	mNrOfReplays = replays;
}

void ImageFileStreamer::setStreamingMode(StreamingMode mode) {
    if(mode == STREAMING_MODE_STORE_ALL_FRAMES && !mMaximumNrOfFramesSet)
        setMaximumNumberOfFrames(0);
    Streamer::setStreamingMode(mode);
}

uint ImageFileStreamer::getNrOfFrames() const {
    return mNrOfFrames;
}

void ImageFileStreamer::setSleepTime(uint milliseconds) {
    mSleepTime = milliseconds;
}

void ImageFileStreamer::setMaximumNumberOfFrames(uint nrOfFrames) {
    mMaximumNrOfFrames = nrOfFrames;
    DynamicData::pointer data = getOutputData<Image>(0);
    data->setMaximumNumberOfFrames(nrOfFrames);
}

void ImageFileStreamer::setTimestampFilename(std::string filepath) {
    mTimestampFilename = filepath;
}

void ImageFileStreamer::execute() {
    getOutputData<Image>(0)->setStreamer(mPtr.lock());
    if(mFilenameFormats.size() == 0)
        throw Exception("No filename format was given to the ImageFileStreamer");
    if(!mStreamIsStarted) {
        // Check that first frame exists before starting streamer

        mStreamIsStarted = true;
        thread = new boost::thread(&stubStreamThread, this);
    }

    // Wait here for first frame
    boost::unique_lock<boost::mutex> lock(mFirstFrameMutex);
    while(!mFirstFrameIsInserted) {
        mFirstFrameCondition.wait(lock);
    }
}

void ImageFileStreamer::setFilenameFormat(std::string str) {
    if(str.find("#") == std::string::npos)
        throw Exception("Filename format must include a hash tag # which will be replaced by a integer starting from 0.");

    mFilenameFormats.clear();
    mFilenameFormats.push_back(str);
}

void ImageFileStreamer::setFilenameFormats(std::vector<std::string> strs) {
    for(std::string str : strs) {
        if(str.find("#") == std::string::npos)
            throw Exception("Filename format must include a hash tag # which will be replaced by a integer starting from 0.");
    }
    mFilenameFormats = strs;
}

void ImageFileStreamer::producerStream() {
    Streamer::pointer pointerToSelf = mPtr.lock(); // try to avoid this object from being destroyed until this function is finished

    // Read timestamp file if available
    std::ifstream timestampFile;
    unsigned long previousTimestamp = 0;
    auto previousTimestampTime = std::chrono::high_resolution_clock::time_point::min();
    if(mTimestampFilename != "") {
        timestampFile.open(mTimestampFilename.c_str());
        if(!timestampFile.is_open()) {
            throw Exception("Timestamp file not found in ImageFileStreamer");
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
        std::string filename = mFilenameFormats[currentSequence];
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
            ImageFileImporter::pointer importer = ImageFileImporter::New();
            importer->setFilename(filename);
            importer->setMainDevice(getMainDevice());
            importer->update();
            Image::pointer image = importer->getOutputData<Image>();
            // Set and use timestamp if available
            if(mTimestampFilename != "") {
                std::string line;
                std::getline(timestampFile, line);
                if(line != "") {
                    unsigned long timestamp = boost::lexical_cast<unsigned long>(line);
                    image->setCreationTimestamp(timestamp);
                    // Wait as long as necessary before adding image
                    auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - previousTimestampTime);
                    //reportInfo() << timestamp << reportEnd();
                    //reportInfo() << previousTimestamp << reportEnd();
                    //reportInfo() << "Time passed: " << timePassed.count() << reportEnd();
                    while(timestamp > previousTimestamp + timePassed.count()) {
                        // Wait
                        boost::this_thread::sleep(boost::posix_time::milliseconds(timestamp-(long)previousTimestamp-timePassed.count()));
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
            DynamicData::pointer ptr = getOutputData<Image>();
            if(ptr.isValid()) {
                try {
                    ptr->addFrame(image);
                    if(mSleepTime > 0)
                        boost::this_thread::sleep(boost::posix_time::milliseconds(mSleepTime));
                } catch(NoMoreFramesException &e) {
                    throw e;
                } catch(Exception &e) {
                    reportInfo() << "streamer has been deleted, stop" << Reporter::end;
                    break;
                }
                if(!mFirstFrameIsInserted) {
                    {
                        boost::lock_guard<boost::mutex> lock(mFirstFrameMutex);
                        mFirstFrameIsInserted = true;
                    }
                    mFirstFrameCondition.notify_one();
                }
            } else {
                reportInfo() << "DynamicImage object destroyed, stream can stop." << Reporter::end;
                break;
            }
            mNrOfFrames++;
            i += mStepSize;
        } catch(FileNotFoundException &e) {
            if(i > 0) {
                reportInfo() << "Reached end of stream" << Reporter::end;
                // If there where no files found at all, we need to release the execute method
                if(!mFirstFrameIsInserted) {
                    {
                        boost::lock_guard<boost::mutex> lock(mFirstFrameMutex);
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

ImageFileStreamer::~ImageFileStreamer() {
    if(mStreamIsStarted) {
        if(thread->get_id() != boost::this_thread::get_id()) { // avoid deadlock
            thread->join();
        }
        delete thread;
    }
}

bool ImageFileStreamer::hasReachedEnd() const {
    return mHasReachedEnd;
}

void ImageFileStreamer::setStartNumber(uint startNumber) {
    mStartNumber = startNumber;
}

void ImageFileStreamer::setZeroFilling(uint digits) {
    mZeroFillDigits = digits;
}

void ImageFileStreamer::enableLooping() {
    mLoop = true;
}

void ImageFileStreamer::disableLooping() {
    mLoop = false;
}

void ImageFileStreamer::setStepSize(uint stepSize) {
    if(stepSize == 0)
        throw Exception("Step size given to ImageFileStreamer can't be 0");
    mStepSize = stepSize;
}

} // end namespace fast
