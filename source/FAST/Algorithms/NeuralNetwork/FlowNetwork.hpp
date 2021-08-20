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
        FAST_CONSTRUCTOR(FlowNetwork)
    protected:
        void execute() override;
};

}
