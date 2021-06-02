#pragma once

#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>

namespace fast {

class TensorToSegmentation;

/**
 * @brief Segmentation neural network process object
 *
 * This class is a convenience class for a neural network which performs segmentation
 * by having 1 input image, and outputs 1 segmentation image. Internally it uses TensorToSegmentation.
 * If you need multi-input or multi-output support, use NeuralNetwork with TensorToSegmentation instead.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT SegmentationNetwork : public NeuralNetwork {
    FAST_OBJECT(SegmentationNetwork)
    public:
        void setHeatmapOutput();
        void setSegmentationOutput();
        /**
         * Threshold to accept a channel X as being class X.
         *
         * @param threshold
         */
        void setThreshold(float threshold);
        float getThreshold() const;
        /**
         * Set whether channel 0 of segmentation tensor is the "background" class, thereby getting the label 0 in the
         * resulting Segmentation.
         *
         * @param hasBackgroundClass
         */
        void setBackgroundClass(bool hasBackgroundClass);
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
