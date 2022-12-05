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
        /**
         * @brief Create instance
         *
         * @param modelFilename path to model to load
         * @param scaleFactor A value which is multiplied with each pixel of input image before it is sent to the neural
         *      network. Use this to scale your pixels values. Default: 1.0
         * @param heatmapOutput If set to true, the output of this object will be a Tensor instead of a segmentation Image. Default: false
         * @param threshold The minimum value of the class confidence value to be accepted. Default: 0.5
         * @param hasBackgroundClass Whether the neural network has a channel 0 which represents the "background".
         * @param meanIntensity Mean intensity to subtract from each pixel of the input image
         * @param standardDeviationIntensity Standard deviation to divide each pixel of the input image by
         * @param inputNodes Specify names, and potentially shapes, of input nodes.
         *      Not necessary unless you only want to use certain inputs or specify the input shape manually.
         * @param outputNodes Specify names, and potentially shapes, of output nodes to use.
         *      Not necessary unless you only want to use certain outputs or specify the output shape manually.
         * @param inferenceEngine Specify which inference engine to use (TensorFlow, TensorRT, OpenVINO).
         *      By default, FAST will select the best inference engine available on your system.
         * @param customPlugins Specify path to any custom plugins/operators to load
         *
         * @return instance
         */
        FAST_CONSTRUCTOR(SegmentationNetwork,
                         std::string, modelFilename,,
                         float, scaleFactor, = 1.0f,
                         bool, heatmapOutput, = false,
                         float, threshold, = 0.5f,
                         bool, hasBackgroundClass, = true,
                         float, meanIntensity, = 0.0f,
                         float, standardDeviationIntensity, = 1.0f,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        );
#ifndef SWIG
        /**
        * @brief Create instance
        * C++ friendly create with parameters that must be set before loading
        *
        * @param modelFilename Path to model to load
        * @param inputNodes Specify names, and potentially shapes, of input nodes.
        *      Not necessary unless you only want to use certain inputs or specify the input shape manually.
        * @param outputNodes Specify names, and potentially shapes, of output nodes to use.
        *      Not necessary unless you only want to use certain outputs or specify the output shape manually.
        * @param inferenceEngine Specify which inference engine to use (TensorFlow, TensorRT, OpenVINO).
        *      By default, FAST will select the best inference engine available on your system.
        * @param customPlugins Specify path to any custom plugins/operators to load
        * @return instance
        */
        FAST_CONSTRUCTOR(SegmentationNetwork,
                         std::string, modelFilename,,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        );
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
        /**
         * @brief Specify list of output channels to ignore
         * @param channels
         */
        void setChannelsToIgnore(std::vector<int> channels);
    private:
        SegmentationNetwork();
        void execute();
        bool mHeatmapOutput = false;
        bool m_resizeBackToOriginalSize = false;
        std::shared_ptr<TensorToSegmentation> m_tensorToSegmentation;

};

}
