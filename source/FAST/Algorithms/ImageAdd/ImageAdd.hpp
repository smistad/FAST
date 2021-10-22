#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Add two images
 */
class FAST_EXPORT  ImageAdd: public ProcessObject {
    FAST_PROCESS_OBJECT(ImageAdd)
    public:
        FAST_CONSTRUCTOR(ImageAdd);
    private:
        void execute();
};

}
