#pragma once

#include "NeuralNetwork.hpp"

namespace fast {

class TensorToSegmentation;

/**
 * This class is a convienence class for a neural network which performs segmentation
 * by having 1 input image, and outputs 1 segmentation image. Internally it uses TensorToSegmentation.
 * If you need multi-input or multi-output support, use NeuralNetwork with TensorToSegmentation instead.
 */
class FAST_EXPORT SegmentationNetwork : public NeuralNetwork {
    FAST_OBJECT(SegmentationNetwork)
    public:
        void setHeatmapOutput();
        void setSegmentationOutput();
        void setThreshold(float threshold);
        void loadAttributes();
        void setResizeBackToOriginalSize(bool resize);
    private:
        SegmentationNetwork();
        void execute();
        bool mHeatmapOutput = false;
        bool m_resizeBackToOriginalSize = false;
        std::shared_ptr<TensorToSegmentation> m_tensorToSegmentation;

};

}
