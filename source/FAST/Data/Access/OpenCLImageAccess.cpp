#include "OpenCLImageAccess.hpp"
using namespace fast;

cl::Image* OpenCLImageAccess::get() const {
    return mImage;
}

cl::Image2D* OpenCLImageAccess::get2DImage() const {
    return (cl::Image2D*)mImage;
}

cl::Image3D* OpenCLImageAccess::get3DImage() const {
    return (cl::Image3D*)mImage;
}


OpenCLImageAccess::OpenCLImageAccess(cl::Image3D* image, bool* accessFlag, bool* accessFlag2) {
    // Copy the image
    mImage = new cl::Image3D(*image);
    mIsDeleted = false;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

OpenCLImageAccess::OpenCLImageAccess(cl::Image2D* image, bool* accessFlag, bool* accessFlag2) {
    // Copy the image
    mImage = new cl::Image2D(*image);
    mIsDeleted = false;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

void OpenCLImageAccess::release() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
    if(!mIsDeleted) {
        delete mImage;
        mImage = new cl::Image3D(); // assign a new blank object
        mIsDeleted = true;
    }
}

OpenCLImageAccess::~OpenCLImageAccess() {
    if(!mIsDeleted)
        release();
}
