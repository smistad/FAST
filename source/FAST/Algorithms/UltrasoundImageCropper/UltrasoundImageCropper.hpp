#ifndef ULTRASOUND_IMAGE_CROPPER_HPP_
#define ULTRASOUND_IMAGE_CROPPER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  UltrasoundImageCropper : public ProcessObject {
    FAST_OBJECT(UltrasoundImageCropper)
    public:
    private:
        UltrasoundImageCropper();
        void execute();
};

}

#endif
