#pragma once

#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>

namespace fast {

class TensorToImage;

/**
 * @brief Image-to-Image neural network process object
 *
 * This class is a convenience class for a neural network which performs image to image transformation
 * by having 1 input image, and outputs 1 image. Internally it uses TensorToImage.
 * If you need multi-input or multi-output support, use NeuralNetwork with TensorToImage instead.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT ImageToImageNetwork : public NeuralNetwork {
    FAST_PROCESS_OBJECT(ImageToImageNetwork)
    public:
        /**
         * @brief Create instance
         *
         * @param modelFilename path to model to load
         * @param scaleFactor A value which is multiplied with each pixel of input image before it is sent to the neural
         *      network. Use this to scale your pixels values. Default: 1.0
         * @param iterations Number of iterations to run the network
         * @param residualNetwork Whether this image-to-image network is a residual network.
         *      If true, the output is added to the input image to create the final output image.
         * @param resizeBackToOriginalSize Whether to resize the output image to its original input image size
         * @param castBackToOriginalType Whether to cast the output image to its input image type
         * @param channelsToExtract Which channels to extract from the output tensor.
         *      Default (empty list) is to extract all channels.
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
        FAST_CONSTRUCTOR(ImageToImageNetwork,
                         std::string, modelFilename,,
                         float, scaleFactor, = 1.0f,
                         int, iterations, = 1,
                         bool, residualNetwork, = false,
                         bool, resizeBackToOriginalSize, = false,
                         bool, castBackToOriginalType, = true,
                         std::vector<int>, channelsToExtract, = std::vector<int>(),
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
        FAST_CONSTRUCTOR(ImageToImageNetwork,
                         std::string, modelFilename,,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        );
#endif
        void loadAttributes();
        void setIterations(int iterations);
        int getIterations() const;
        void setResidualNetwork(bool residual);
        void setResizeOutput(bool resizeOutput);
        void setCastOutput(bool castOutput);
        void setChannels(std::vector<int> channels);
    private:
        ImageToImageNetwork();
        void execute();
        void init();
        std::shared_ptr<NeuralNetwork> m_network;
        std::shared_ptr<TensorToImage> m_tensorToImage;
        int m_iterations = 1;
        bool m_residualNetwork = true;
        bool m_resizeBackToOriginalSize = false;
        bool m_castBackToOriginalType = true;
        std::vector<int> m_channelsToExtract;
        float mScaleFactor2 = 1.0f;

};

}
