#ifndef FAST_IMAGE_MULTIPLY_HPP_
#define FAST_IMAGE_MULTIPLY_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class ImageMultiply : public ProcessObject {
    FAST_OBJECT(ImageMultiply)
    public:
    private:
        ImageMultiply();
        void execute();
};

}

#endif