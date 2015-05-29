#ifndef META_IMAGE_STREAMER_HPP
#define META_IMAGE_STREAMER_HPP

#include "FAST/SmartPointers.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <boost/thread.hpp>

namespace fast {

class ImageFileStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(ImageFileStreamer)
    public:
        void setFilenameFormat(std::string str);
        void setStartNumber(uint startNumber);
        void setStepSize(uint step);
        void setZeroFilling(uint digits);
        void setStreamingMode(StreamingMode mode);
        void setMaximumNumberOfFrames(uint nrOfFrames);
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
        uint mZeroFillDigits;
        uint mStartNumber;
        uint mNrOfFrames;
        uint mMaximumNrOfFrames;
        bool mMaximumNrOfFramesSet;
        uint mSleepTime;
        uint mStepSize;

        boost::thread *thread;
        boost::mutex mFirstFrameMutex;
        boost::condition_variable mFirstFrameCondition;

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;

        std::string mFilenameFormat;

};

} // end namespace fast

#endif
