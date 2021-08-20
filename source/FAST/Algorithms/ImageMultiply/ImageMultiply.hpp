#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Multiply two images
 */
class FAST_EXPORT  ImageMultiply : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageMultiply)
    public:
        FAST_CONSTRUCTOR(ImageMultiply)
    private:
        void execute();
};

}
