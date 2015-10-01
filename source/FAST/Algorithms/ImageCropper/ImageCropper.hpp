#ifndef IMAGE_CROPPER_HPP_
#define IMAGE_CROPPER_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class ImageCropper : public ProcessObject {
    FAST_OBJECT(ImageCropper)
    public:
        void setOffset(VectorXui offset);
        void setSize(VectorXui size);
    private:
        ImageCropper();
        void execute();

        VectorXui mOffset;
        VectorXui mSize;
};


} // end namespace fast

#endif
