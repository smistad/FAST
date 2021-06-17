#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

enum class BoundingBoxNetworkType {
    YOLOv3,
};

class FAST_EXPORT TensorToBoundingBoxSet : public ProcessObject {
    FAST_PROCESS_OBJECT(TensorToBoundingBoxSet)
    public:
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