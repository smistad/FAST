#pragma once

#include "NeuralNetwork.hpp"

namespace fast {
class FAST_EXPORT BoundingBoxNetwork : public NeuralNetwork {
    FAST_OBJECT(BoundingBoxNetwork)
    public:
        void setThreshold(float threshold);
        void loadAttributes() override;
    private:
        BoundingBoxNetwork();
        void execute() override;

        float m_threshold;

};

}
