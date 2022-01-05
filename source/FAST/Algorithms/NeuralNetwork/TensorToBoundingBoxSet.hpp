#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

/**
 * @brief Type of bounding box network
 * Used to determine how the output tensor of a bounding box network should be converted to a set of bounding boxes.
 *
 * @ingroup neural-network bounding-box
 */
enum class BoundingBoxNetworkType {
    YOLOv3,
};

/**
 * @brief Convert a tensor to a set of bounding boxes
 *
 * Used in BoundingBoxNetwork to convert a tensor to a set of bounding boxes.
 *
 * @sa BoundingBoxNetwork BoundingBoxNetworkType
 * @ingroup neural-network bounding-box
 */
class FAST_EXPORT TensorToBoundingBoxSet : public ProcessObject {
    FAST_PROCESS_OBJECT(TensorToBoundingBoxSet)
    public:
        /**
         * @brief Create instance
         *
         * @param type Type of bounding box detection network. Used to determine how the output tensor should be handled to create bounding boxes. Default: YOLOv3
         * @param threshold Threshold for how high score a bounding box need to be accepted.
         * @param anchors List of anchors to use when calculating bounding boxes parameters from the output tensor.
         * @return instance
         */
        FAST_CONSTRUCTOR(TensorToBoundingBoxSet,
                         BoundingBoxNetworkType, type, = BoundingBoxNetworkType::YOLOv3,
                         float, threshold, = 0.5f,
                         std::vector<std::vector<Vector2f>>, anchors, = std::vector<std::vector<Vector2f>>()
         )
        void setType(BoundingBoxNetworkType type);
        void setThreshold(float threshold);
        void setAnchors(std::vector<std::vector<Vector2f>> anchors);
        void setInputConnection(DataChannel::pointer channel) override;
        void setInputConnection(uint portID, DataChannel::pointer channel) override;
        void setNrOfInputNodes(int nr);
        void loadAttributes() override;
    private:
        void execute() override;

        BoundingBoxNetworkType m_type;
        std::vector<std::vector<Vector2f>> m_anchors;
        float m_threshold = 0.5;
};

}