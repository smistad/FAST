#ifndef FAST_FILE_STREAMER_HPP_
#define FAST_FILE_STREAMER_HPP_

#include "FAST/ProcessObject.hpp"

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

/**
 * Abstract FileStreamer class
 */
class FAST_EXPORT  FileStreamer : public Streamer {
    public:
        void setFilenameFormat(std::string str);
        void setFilenameFormats(std::vector<std::string> strings);
        void setStartNumber(uint startNumber);
        void setStepSize(uint step);
        void setZeroFilling(uint digits);
        void setNumberOfReplays(uint replays);
        void setMaximumNumberOfFrames(uint nrOfFrames);
        void setTimestampFilename(std::string filepath);
        void enableLooping();
        void disableLooping();
        /**
         * Set a sleep time after each frame is read
         */
        void setSleepTime(uint milliseconds);
        bool hasReachedEnd();
        int getNrOfFrames();
        void producerStream();
        /**
         * Stops the streaming thread, and will not return until this thread is stopped.
         */
        void stop();

        /**
         * Enable or disable the use of timestamps when streaming files.
         *
         * @param use
         */
        void setUseTimestamp(bool use);

        ~FileStreamer();
    protected:
        virtual DataObject::pointer getDataFrame(std::string filename) = 0;
        std::string getFilename(uint i, int currentSequence) const;
        FileStreamer();
    private:
        void execute();

        bool mLoop;
        int mNrOfReplays;
        uint mZeroFillDigits;
        int mStartNumber;
        int mNrOfFrames;
        int mMaximumNrOfFrames;
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
        bool mUseTimestamp = true;

        std::vector<std::string> mFilenameFormats;
        std::string mTimestampFilename;


};

}

#endif
