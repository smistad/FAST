#ifndef IMAGE_CROPPER_HPP_
#define IMAGE_CROPPER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class FAST_EXPORT  ImageCropper : public ProcessObject {
    FAST_OBJECT(ImageCropper)
    public:
        void setOffset(VectorXi offset);
        void setSize(VectorXi size);
        /*
         * If out of bounds cropping is allowed, offset can be negative and size can be larger than image.
         * Any pixels outside of image will be replaced with 0.
         */
        void allowOutOfBoundsCropping(bool allow);
    private:
        ImageCropper();
        void execute();

        VectorXi mOffset;
        VectorXi mSize;
        bool mAllowOutOfBoundsCropping;
};


} // end namespace fast

#endif
