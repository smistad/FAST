#pragma once

#include "FAST/ProcessObject.hpp"

#include <FAST/Streamers/Streamer.hpp>
#include <thread>

namespace fast {

/**
 * Abstract FileStreamer class
 */
class FAST_EXPORT FileStreamer : public Streamer {
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
        int getNrOfFrames();

        /**
         * Enable or disable the use of timestamps when streaming files.
         *
         * @param use
         */
        void setUseTimestamp(bool use);

        ~FileStreamer();

        virtual std::string getNameOfClass() const { return "FileStreamer"; }

        void loadAttributes() override;
    protected:
        virtual DataObject::pointer getDataFrame(std::string filename) = 0;
        std::string getFilename(uint i, int currentSequence) const;
        void generateStream() override;
        FileStreamer();
        void execute();

        bool mLoop;
        int mNrOfReplays;
        uint mZeroFillDigits;
        int mStartNumber;
        int mNrOfFrames;
        int mMaximumNrOfFrames;
        uint mSleepTime;
        uint mStepSize;

        bool mUseTimestamp = true;

        std::vector<std::string> mFilenameFormats;
        std::string mTimestampFilename;


};

}
