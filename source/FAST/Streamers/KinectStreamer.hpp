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

/**
 * \brief Streams data RGB and depth data from a kinect device.
 *
 * The RGB camera and depth stream are registered so that a color value for each point in the
 * point cloud is established.
 *
 * Output port 0: Registered RGB image
 * Output port 1: Registered depth image
 * Output port 2: Registered point cloud
 */
class FAST_EXPORT KinectStreamer : public Streamer, public ProcessObject {
    FAST_OBJECT(KinectStreamer);
    public:
        void producerStream();
        void setPointCloudFiltering(bool enabled);
        /**
         * Set maximum range in meters. All points above this range will be dropped.
         * @param range
         */
        void setMaxRange(float range);
        /**
         * Set minimum range in meters. All points below this range will be dropped.
         * @param range
         */
        void setMinRange(float range);
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
        bool mPointCloudFilterEnabled;
        float mMaxRange = std::numeric_limits<float>::max(), mMinRange = 0;
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
