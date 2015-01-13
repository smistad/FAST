#ifndef IMAGE_GRADIENT_HPP
#define IMAGE_GRADIENT_HPP

#include "ProcessObject.hpp"
#include "Image.hpp"

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
