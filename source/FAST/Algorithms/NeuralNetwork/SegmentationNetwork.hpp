#pragma once

#include "NeuralNetwork.hpp"

namespace fast {
class FAST_EXPORT SegmentationNetwork : public NeuralNetwork {
    FAST_OBJECT(SegmentationNetwork)
    public:
        void setHeatmapOutput();
        void setSegmentationOutput();
        void setResizeBackToOriginalSize(bool resize);
        void setThreshold(float threshold);
        void loadAttributes();
    private:
        SegmentationNetwork();
        void execute();


        bool mHeatmapOutput;
        bool mResizeBackToOriginalSize;
        float mThreshold;

};

}
