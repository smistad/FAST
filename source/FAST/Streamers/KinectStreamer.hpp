#ifndef FAST_KINECT_STREAMER_HPP_
#define FAST_KINECT_STREAMER_HPP_

#include "FAST/ProcessObject.hpp"
#include "Streamer.hpp"
#include <thread>

namespace libfreenect2 {
class Frame;
class Registration;
}

namespace fast {

class MeshVertex;

class KinectStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(KinectStreamer);
    public:
        void producerStream();
        bool hasReachedEnd() const;
        uint getNrOfFrames() const;
        /**
         * Gets corresponding 3D point from rgb image coordinate
         * @param x
         * @param y
         * @return
         */
        MeshVertex getPoint(int x, int y);
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
        libfreenect2::Registration* registration;
        libfreenect2::Frame* mUndistorted;
        libfreenect2::Frame* mRegistered;
};

}

#endif