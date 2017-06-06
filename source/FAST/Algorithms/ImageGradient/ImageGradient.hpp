#ifndef IMAGE_GRADIENT_HPP
#define IMAGE_GRADIENT_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

class FAST_EXPORT  ImageGradient : public ProcessObject {
    FAST_OBJECT(ImageGradient);
    public:
        /**
         * Use 16 bit format to reduce memory usage
         */
        void set16bitStorageFormat();
        /**
         * Use regular 32 bit float format (default)
         */
        void set32bitStorageFormat();
    private:
        ImageGradient();
        void execute();

        bool mUse16bitFormat;
};

}


#endif
