#pragma once

#include "FAST/ProcessObject.hpp"
#include "Streamer.hpp"
#include <thread>
#include <stack>

struct rs2_intrinsics;

namespace fast {

class Image;
class MeshVertex;

/**
 * @brief Streams data RGB and depth data from an Intel RealSense camera
 *
 * The RGB camera and depth stream are registered so that a color value for each point in the
 * point cloud is established.
 *
 * Default streaming mode is StreamingMode::NewestFrameOnly
 *
 * <h3>Output ports</h3>
 * - 0: Image - Registered RGB image
 * - 1: Image - Registered depth image
 * - 2: Mesh - Registered point cloud
 *
 * @ingroup streamers
 */
class FAST_EXPORT RealSenseStreamer : public Streamer {
    FAST_PROCESS_OBJECT(RealSenseStreamer);
    public:
        FAST_CONSTRUCTOR(RealSenseStreamer)
        /**
         * Set maximum range in millimeters. All points above this range will be dropped.
         * @param range
         */
        void setMaxRange(float range);
        /**
         * Set minimum range in millimeters. All points below this range will be dropped.
         * @param range
         */
        void setMinRange(float range);

        void setMaxWidth(float range);
        void setMinWidth(float range);
        void setMaxHeight(float range);
        void setMinHeight(float range);

        uint getNrOfFrames() const;
        /**
         * Gets corresponding 3D point from rgb image coordinate
         * @param x
         * @param y
         * @return
         */
        MeshVertex getPoint(int x, int y);
        ~RealSenseStreamer();
    private:
        void execute();
        void generateStream() override;

        float mMaxRange = std::numeric_limits<float>::max();
        float mMinRange = 0;
        float mMaxWidth = std::numeric_limits<float>::max();
        float mMinWidth = -std::numeric_limits<float>::max();
        float mMaxHeight = std::numeric_limits<float>::max();
        float mMinHeight = -std::numeric_limits<float>::max();

        uint mNrOfFrames;

        rs2_intrinsics* intrinsics;
        std::shared_ptr<Image> mDepthImage;
        std::shared_ptr<Image> mColorImage;
};

}
