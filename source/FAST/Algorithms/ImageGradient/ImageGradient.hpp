#pragma once

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

/**
 * @brief Calculate image gradient using central finite difference method
 *
 * @ingroup filter
 */
class FAST_EXPORT  ImageGradient : public ProcessObject {
    FAST_PROCESS_OBJECT(ImageGradient);
    public:
        FAST_CONSTRUCTOR(ImageGradient, bool, use16BitFormat, = false);
        /**
         * Use 16 bit format to reduce memory usage
         */
        void set16bitStorageFormat();
        /**
         * Use regular 32 bit float format (default)
         */
        void set32bitStorageFormat();
    private:
        void execute();

        bool mUse16bitFormat;
};

}