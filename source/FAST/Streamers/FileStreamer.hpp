#ifndef FAST_FILE_STREAMER_HPP_
#define FAST_FILE_STREAMER_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/SmartPointers.hpp"
#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

/**
 * Abstract FileStreamer class
 */
class FAST_EXPORT  FileStreamer : public Streamer, public ProcessObject {
    public:
        void setFilenameFormat(std::string str);
        void setFilenameFormats(std::vector<std::string> strings);
        void setStartNumber(uint startNumber);
        void setStepSize(uint step);
        void setZeroFilling(uint digits);
        void setNumberOfReplays(uint replays);
        void setStreamingMode(StreamingMode mode);
        void setMaximumNumberOfFrames(uint nrOfFrames);
        void setTimestampFilename(std::string filepath);
        void enableLooping();
        void disableLooping();
        /**
         * Set a sleep time after each frame is read
         */
        void setSleepTime(uint milliseconds);
        bool hasReachedEnd() const;
        uint getNrOfFrames() const;
        void producerStream();
        /**
         * Stops the streaming thread, and will not return until this thread is stopped.
         */
        void stop();

        ~FileStreamer();
    protected:
        virtual DataObject::pointer getDataFrame(std::string filename) = 0;
        FileStreamer();
    private:
        void execute();

        bool mLoop;
        int mNrOfReplays;
        uint mZeroFillDigits;
        uint mStartNumber;
        uint mNrOfFrames;
        uint mMaximumNrOfFrames;
        bool mMaximumNrOfFramesSet;
        uint mSleepTime;
        uint mStepSize;

        std::thread *mThread;
        std::mutex mFirstFrameMutex;
        std::condition_variable mFirstFrameCondition;
        std::mutex mStopMutex;

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;
        bool mStop;

        std::vector<std::string> mFilenameFormats;
        std::string mTimestampFilename;



};

}

#endif
