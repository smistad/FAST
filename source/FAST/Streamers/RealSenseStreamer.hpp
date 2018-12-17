#ifndef FAST_KINECT_STREAMER_HPP_
#define FAST_KINECT_STREAMER_HPP_

#include "FAST/ProcessObject.hpp"
#include "Streamer.hpp"
#include <thread>
#include <stack>

struct rs2_intrinsics;

namespace fast {

class Image;
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
class FAST_EXPORT RealSenseStreamer : public Streamer {
    FAST_OBJECT(RealSenseStreamer);
    public:
        void producerStream();
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
        bool hasReachedEnd();
        uint getNrOfFrames() const;
        /**
         * Gets corresponding 3D point from rgb image coordinate
         * @param x
         * @param y
         * @return
         */
        MeshVertex getPoint(int x, int y);
        ~RealSenseStreamer();
        void stop();
    private:
        RealSenseStreamer();

        void execute();

        bool mStreamIsStarted;
        bool mFirstFrameIsInserted;
        bool mHasReachedEnd;
        bool mStop;
        float mMaxRange = std::numeric_limits<float>::max();
        float mMinRange = 0;
        uint mNrOfFrames;

        std::unique_ptr<std::thread> mThread;
        std::mutex mFirstFrameMutex;
        std::mutex mStopMutex;
        std::condition_variable mFirstFrameCondition;

        rs2_intrinsics* intrinsics;
        SharedPointer<Image> mDepthImage;
        SharedPointer<Image> mColorImage;
};

}

#endif
