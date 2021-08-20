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
        FAST_CONSTRUCTOR(BoundingBoxNetwork,
                         std::string, modelFilename,,
                         float, scaleFactor, = 1.0f,
                         float, threshold,= 0.5f,
                         std::vector<std::vector<Vector2f>>, anchors, = std::vector<std::vector<Vector2f>>(),
                         BoundingBoxNetworkType, type, = BoundingBoxNetworkType::YOLOv3,
                         float, meanIntensity, = 0.0f,
                         float, stanardDeviationIntensity, = 1.0f,
                         std::vector<NeuralNetworkNode>, inputNodes, = std::vector<NeuralNetworkNode>(),
                         std::vector<NeuralNetworkNode>, outputNodes, = std::vector<NeuralNetworkNode>(),
                         std::string, inferenceEngine, = "",
                         std::vector<std::string>, customPlugins, = std::vector<std::string>()
                         )
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
