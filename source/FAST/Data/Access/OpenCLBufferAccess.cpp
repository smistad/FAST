#include "OpenCLBufferAccess.hpp"
#include <iostream>
#include "FAST/Data/Image.hpp"
#include "FAST/ExecutionDevice.hpp"

namespace fast {

cl::Buffer* OpenCLBufferAccess::get() const {
    return mBuffer;
}

OpenCLBufferAccess::OpenCLBufferAccess(cl::Buffer* buffer,  SharedPointer<Image> image) {
    // Copy the image
    mBuffer = new cl::Buffer(*buffer);
    mIsDeleted = false;
    mImage = image;
}

void OpenCLBufferAccess::release() {
    if(!mIsDeleted) {
        delete mBuffer;
        mBuffer = nullptr;
        mIsDeleted = true;
    }
	mImage->accessFinished();
}

OpenCLBufferAccess::~OpenCLBufferAccess() {
    if(!mIsDeleted)
        release();
}

}
