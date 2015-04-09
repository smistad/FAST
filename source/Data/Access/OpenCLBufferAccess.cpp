#include "OpenCLBufferAccess.hpp"
#include <iostream>
using namespace fast;

cl::Buffer* OpenCLBufferAccess::get() const {
    return mBuffer;
}

OpenCLBufferAccess::OpenCLBufferAccess(cl::Buffer* buffer, bool* accessFlag, bool* accessFlag2) {
    // Copy the image
    mBuffer = new cl::Buffer(*buffer);
    mIsDeleted = false;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

void OpenCLBufferAccess::release() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
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


