#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Add two images
 */
class FAST_EXPORT  ImageAdd: public ProcessObject {
    FAST_PROCESS_OBJECT(ImageAdd)
    public:
        /**
         * @brief Create instance
         * @return instance
         */
        FAST_CONSTRUCTOR(ImageAdd);
    private:
        void execute();
};

}
