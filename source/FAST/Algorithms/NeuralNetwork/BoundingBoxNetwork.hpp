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
 * @ingroup neural-network
 */
class FAST_EXPORT BoundingBoxNetwork : public NeuralNetwork {
    FAST_OBJECT(BoundingBoxNetwork)
    public:
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
