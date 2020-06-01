#pragma once

#include "NeuralNetwork.hpp"

namespace fast {
class FAST_EXPORT BoundingBoxNetwork : public NeuralNetwork {
    FAST_OBJECT(BoundingBoxNetwork)
    public:
        void setThreshold(float threshold);
        void loadAttributes() override;
        void setAnchors(std::vector<std::vector<Vector2f>> anchors);
    private:
        BoundingBoxNetwork();
        void execute() override;

        float m_threshold;
        std::vector<std::vector<Vector2f>> m_anchors;


};

}
