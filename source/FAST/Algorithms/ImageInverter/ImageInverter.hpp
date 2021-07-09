#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Invert the intensity of an image
 */
class FAST_EXPORT  ImageInverter : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageInverter)
    public:
        FAST_CONSTRUCTOR(ImageInverter)
    private:
        void execute();
};

}