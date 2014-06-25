#include "OpenCLImageAccess2D.hpp"
using namespace fast;

cl::Image2D* OpenCLImageAccess2D::get() const {
    return mImage;
}

OpenCLImageAccess2D::OpenCLImageAccess2D(cl::Image2D * image, bool* accessFlag, bool* accessFlag2) {
    // Copy the image
    mImage = new cl::Image2D(*image);
    mIsDeleted = false;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

void OpenCLImageAccess2D::release() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
    if(!mIsDeleted) {
        delete mImage;
        mImage = new cl::Image2D(); // assign a new blank object
        mIsDeleted = true;
    }
}

OpenCLImageAccess2D::~OpenCLImageAccess2D() {
    if(!mIsDeleted)
        release();
}
