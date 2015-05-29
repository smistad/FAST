#ifndef IMAGE_GRADIENT_HPP
#define IMAGE_GRADIENT_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class ImageGradient : public ProcessObject {
    FAST_OBJECT(ImageGradient);
    public:
    private:
        ImageGradient();
        void execute();
};

}


#endif
