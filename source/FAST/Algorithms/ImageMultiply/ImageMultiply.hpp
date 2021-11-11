#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Multiply two images
 */
class FAST_EXPORT  ImageMultiply : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageMultiply)
    public:
        /**
         * @brief Create instance
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageMultiply);
    private:
        void execute();
};

}
