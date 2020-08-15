#include "OpenCLBufferAccess.hpp"
#include <iostream>
#include "FAST/Data/Image.hpp"
#include "FAST/ExecutionDevice.hpp"

namespace fast {

cl::Buffer* OpenCLBufferAccess::get() const {
    return mBuffer;
}

OpenCLBufferAccess::OpenCLBufferAccess(cl::Buffer* buffer,  std::shared_ptr<DataObject> dataObject) {
    // Copy the image
    mBuffer = new cl::Buffer(*buffer);
    mIsDeleted = false;
    mDataObject = dataObject;
}

void OpenCLBufferAccess::release() {
    if(!mIsDeleted) {
        delete mBuffer;
        mBuffer = nullptr;
        mIsDeleted = true;
    }
	mDataObject->accessFinished();
}

OpenCLBufferAccess::~OpenCLBufferAccess() {
    if(!mIsDeleted)
        release();
}

}
