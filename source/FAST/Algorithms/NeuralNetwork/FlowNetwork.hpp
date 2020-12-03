#pragma once

#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <deque>

namespace fast {

class Image;

/**
 * This process object converts a stream of images to a stream of sequences of a given size.
 * If the sequence size is X, for the first execute, the added item will be copied X times in the sequence.
 * New images are added to the end/back of the sequence, while the first/oldest image, is removed.
 */
class FAST_EXPORT ImagesToSequence : public ProcessObject {
    FAST_OBJECT(ImagesToSequence)
    public:
        void setSequenceSize(int size);
        void loadAttributes() override;
    protected:
        ImagesToSequence();
        void execute() override;

        int m_sequenceSize = 2;
        std::deque<std::shared_ptr<Image>> m_queue;
};

/**
 * A neural network which takes a sequence of images as input, and then
 * outputs a flow/displacement vector field. This can be a flow network which
 * estimates the motion between two frames.
 */
class FAST_EXPORT FlowNetwork : public NeuralNetwork {
    FAST_OBJECT(FlowNetwork)
    public:
    protected:
        void execute() override;
        FlowNetwork();
};

}
