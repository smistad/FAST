#ifndef MANUAL_IMAGE_STREAMER_HPP
#define MANUAL_IMAGE_STREAMER_HPP

#include "FAST/SmartPointers.hpp"
#include "FAST/Streamers/Streamer.hpp"
#include "FAST/ProcessObject.hpp"
#include <thread>

namespace fast {

class Image;

class FAST_EXPORT  ManualImageStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(ManualImageStreamer)
    public:
    	void addImage(SharedPointer<Image> image);
        void setStartNumber(uint startNumber);
        void setStepSize(uint step);
        void setZeroFilling(uint digits);
        void setNumberOfReplays(uint replays);
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

        ~ManualImageStreamer();
    private:
        ManualImageStreamer();

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

        std::vector<SharedPointer<Image> > mImages;
};

} // end namespace fast

#endif
