#ifndef FAST_KINECT_STREAMER_HPP_
#define FAST_KINECT_STREAMER_HPP_

#include "FAST/ProcessObject.hpp"
#include "Streamer.hpp"
#include <thread>

namespace fast {

class KinectStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(KinectStreamer);
    public:
        void producerStream();
        bool hasReachedEnd() const;
        uint getNrOfFrames() const;
        ~KinectStreamer();
        void stop();
    private:
        KinectStreamer();

        void execute();

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;
        bool mStop;
        uint mNrOfFrames;

        std::thread* mThread;
        std::mutex mFirstFrameMutex;
        std::mutex mStopMutex;
        std::condition_variable mFirstFrameCondition;
};

}

#endif