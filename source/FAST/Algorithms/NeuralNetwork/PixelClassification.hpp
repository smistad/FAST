#ifndef FAST_PIXEL_CLASSIFICATION_HPP_
#define FAST_PIXEL_CLASSIFICATION_HPP_

#include "NeuralNetwork.hpp"

namespace fast {
class PixelClassification : public NeuralNetwork {
    FAST_OBJECT(PixelClassification)
    public:
    private:
        void execute();

};

}

#endif