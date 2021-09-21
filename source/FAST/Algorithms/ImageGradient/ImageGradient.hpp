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
        /**
         * @brief Create instance
         * @param use16BitFormat Use normalized 16 bit float representation instead of regular 32 bit
         * @return instance
         */
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