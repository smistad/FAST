#ifndef IMAGE_TO_IMAGE_NETWORK_HPP_
#define IMAGE_TO_IMAGE_NETWORK_HPP_

#include "NeuralNetwork.hpp"

namespace fast {

class FAST_EXPORT ImageToImageNetwork : public NeuralNetwork {
    FAST_OBJECT(ImageToImageNetwork);
    public:
    private:
        ImageToImageNetwork();
        void execute();
};

}

#endif
