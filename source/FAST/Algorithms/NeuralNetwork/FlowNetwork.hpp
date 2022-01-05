#pragma once

#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <deque>

namespace fast {

class Image;

/**
 * @brief Process object for converting a stream of images to a Sequence data object
 *
 * This process object converts a stream of images to a stream of sequences of a given size.
 * If the sequence size is X, for the first execute, the added item will be copied X times in the sequence.
 * New images are added to the end/back of the sequence, while the first/oldest image, is removed.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT ImagesToSequence : public ProcessObject {
    FAST_PROCESS_OBJECT(ImagesToSequence)
    public:
        /**
         * @brief Create instance
         * @param sequenceSize Set size of sequence
         * @return instance
         */
        FAST_CONSTRUCTOR(ImagesToSequence,
                         int, sequenceSize, = 2
        )
        void setSequenceSize(int size);
        void loadAttributes() override;
    protected:
        void execute() override;

        int m_sequenceSize = 2;
        std::deque<std::shared_ptr<Image>> m_queue;
};

/**
 * @brief A neural network for optical flow estimation
 *
 * A neural network which takes a sequence of images as input, and then
 * outputs a flow/displacement vector field. This can be a flow network which
 * estimates the motion between two frames.
 *
 * @ingroup neural-network
 */
class FAST_EXPORT FlowNetwork : public NeuralNetwork {
    FAST_PROCESS_OBJECT(FlowNetwork)
    public:
        /**
        * @brief Create instance
        * Python friendly constructor with almost all parameters.
        *
        * @param modelFilename Path to model to load
        * @param scaleFactor A value which is multiplied with each pixel of input image before it is sent to the neural
        *      network. Use this to scale your pixels values. Default: 1.0
        * @param meanIntensity Mean intensity to subtract from each pixel of the input image
        * @param standardDeviationIntensity Standard deviation to divide each pixel of the input image by
        * @param inputNodes Specify names, and potentially shapes, of input nodes.
        *      Not necessary unless you only want to use certain inputs or specify the input shape manually.
        * @param outputNodes Specify names, and potentially shapes, of output nodes to use.
        *      Not necessary unless you only want to use certain outputs or specify the output shape manually.
        * @param inferenceEngine Specify which inference engine to use (TensorFlow, TensorRT, OpenVINO).
        *      By default, FAST will select the best inference engine available on your system.
        * @param customPlugins Specify path to any custom plugins/operators to load
        * @return instance
        */
        FAST_CONSTRUCTOR(FlowNetwork,
                     std::string, modelFilename,,
                     float, scaleFactor, = 1.0f,
                     float, meanIntensity, = 0.0f,
                     float, stanardDeviationIntensity, = 1.0f,
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
        FAST_CONSTRUCTOR(FlowNetwork,
                         std::string, modelFilename,,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        );
#endif
    protected:
        FlowNetwork();
        void execute() override;
    };

}
