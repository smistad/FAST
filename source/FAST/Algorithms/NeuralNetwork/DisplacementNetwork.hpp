#pragma once

#include <FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp>
#include <deque>

namespace fast {

class Image;

class FAST_EXPORT ImagesToSequence : public ProcessObject {
    FAST_OBJECT(ImagesToSequence)
    public:
        void setSequenceSize(int size);
    protected:
        ImagesToSequence();
        void execute() override;

        int m_sequenceSize = 2;
        std::deque<std::shared_ptr<Image>> m_queue;
};

/**
 * A neural network which takes a sequence of images as input, and then
 * outputs a displacement vector field. This can be a flow network which
 * estimates the motion between two frames.
 */
class FAST_EXPORT DisplacementNetwork : public NeuralNetwork {
    FAST_OBJECT(DisplacementNetwork)
    public:
    protected:
        void execute() override;
        DisplacementNetwork();
};

}