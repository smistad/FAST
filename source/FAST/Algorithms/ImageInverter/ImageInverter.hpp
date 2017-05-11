#ifndef IMAGE_INVERTER_HPP_
#define IMAGE_INVERTER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  ImageInverter : public ProcessObject {
    FAST_OBJECT(ImageInverter)
    public:
    private:
        ImageInverter();
        void execute();
};

}

#endif