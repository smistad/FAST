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
    FAST_PROCESS_OBJECT(SegmentationNetwork)
    public:
        FAST_CONSTRUCTOR(SegmentationNetwork,
                         std::string, modelFilename,,
                         float, scaleFactor, = 1.0f,
                         bool, heatmapOutput, = false,
                         float, threshold, = 0.5f,
                         bool, hasBackgroundClass, = true,
                         float, meanIntensity, = 0.0f,
                         float, stanardDeviationIntensity, = 1.0f,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        )
#ifndef SWIG
        // C++ friendly create with parameters that must be set before loading
        FAST_CONSTRUCTOR(SegmentationNetwork,
                         std::string, modelFilename,,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        )
#endif
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
