#ifndef META_IMAGE_STREAMER_HPP
#define META_IMAGE_STREAMER_HPP

#include "FAST/SmartPointers.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <thread>

namespace fast {

class ImageFileStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(ImageFileStreamer)
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
        /**
         * This method runs in a separate thread and adds frames to the
         * output object
         */
        void producerStream();

        ~ImageFileStreamer();
    private:
        ImageFileStreamer();

        // Update the streamer if any parameters have changed
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

        std::thread *thread;
        std::mutex mFirstFrameMutex;
        std::condition_variable mFirstFrameCondition;

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;

        std::vector<std::string> mFilenameFormats;
        std::string mTimestampFilename;

};

} // end namespace fast

#endif
