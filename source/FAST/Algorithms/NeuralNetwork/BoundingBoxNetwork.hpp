#pragma once

#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <FAST/Algorithms/NeuralNetwork/TensorToBoundingBoxSet.hpp>

namespace fast {

class TensorToBoundingBoxSet;

/**
 * @brief Neural network process object for bounding box detection
 *
 * This class is a convenience class for a neural network which performs bounding box prediction.
 *
 * @ingroup neural-network bounding-box
 */
class FAST_EXPORT BoundingBoxNetwork : public NeuralNetwork {
    FAST_PROCESS_OBJECT(BoundingBoxNetwork)
    public:
    /**
     * @brief Create instance
     *
     * @param modelFilename Path to model to load
     * @param scaleFactor A value which is multiplied with each pixel of input image before it is sent to the neural
     *      network. Use this to scale your pixels values. Default: 1.0
     * @param threshold Threshold for how high score a bounding box need to be accepted.
     * @param anchors List of anchors to use when calculating bounding boxes from the output tensor.
     * @param type Type of bounding box detection network. Used to determine how the output tensor should be handled to create bounding boxes. Default: YOLOv3
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
        FAST_CONSTRUCTOR(BoundingBoxNetwork,
                         std::string, modelFilename,,
                         float, scaleFactor, = 1.0f,
                         float, threshold,= 0.5f,
                         std::vector<std::vector<Vector2f>>, anchors, = std::vector<std::vector<Vector2f>>(),
                         BoundingBoxNetworkType, type, = BoundingBoxNetworkType::YOLOv3,
                         float, meanIntensity, = 0.0f,
                         float, standardDeviationIntensity, = 1.0f,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
        );
        void setThreshold(float threshold);
        void loadAttributes() override;
        void setAnchors(std::vector<std::vector<Vector2f>> anchors);
        void setType(BoundingBoxNetworkType type);
    private:
        BoundingBoxNetwork();
        void execute() override;

        std::shared_ptr<TensorToBoundingBoxSet> m_tensorToBoundingBoxSet;
};

}
