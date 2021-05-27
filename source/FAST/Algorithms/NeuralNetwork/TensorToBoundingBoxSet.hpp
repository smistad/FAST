#pragma once

#include <FAST/ProcessObject.hpp>

namespace fast {

enum class BoundingBoxNetworkType {
    YOLOv3,
};

class FAST_EXPORT TensorToBoundingBoxSet : public ProcessObject {
    FAST_OBJECT(TensorToBoundingBoxSet)
    public:
        void setType(BoundingBoxNetworkType type);
        void setThreshold(float threshold);
        void setAnchors(std::vector<std::vector<Vector2f>> anchors);
        void setInputConnection(DataChannel::pointer channel) override;
        void setInputConnection(uint portID, DataChannel::pointer channel) override;
        void setNrOfInputNodes(int nr);
        void loadAttributes() override;
    private:
        TensorToBoundingBoxSet();
        void execute() override;

        BoundingBoxNetworkType m_type;
        std::vector<std::vector<Vector2f>> m_anchors;
        float m_threshold = 0.5;
};

}