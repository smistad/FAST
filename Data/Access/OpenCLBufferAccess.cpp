#include "OpenCLBufferAccess.hpp"
using namespace fast;

cl::Buffer* OpenCLBufferAccess::get() const {
    return mBuffer;
}

OpenCLBufferAccess::OpenCLBufferAccess(cl::Buffer* buffer, bool* accessFlag) {
    // Copy the image
    mBuffer = new cl::Buffer(*buffer);
    mIsDeleted = false;
    mAccessFlag = accessFlag;
}

void OpenCLBufferAccess::release() {
    *mAccessFlag = false;
    if(!mIsDeleted) {
        delete mBuffer;
        mBuffer = new cl::Buffer(); // assign a new blank object
        mIsDeleted = true;
    }
}

OpenCLBufferAccess::~OpenCLBufferAccess() {
    if(!mIsDeleted)
        release();
}


