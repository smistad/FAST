#include "OpenCLImageAccess3D.hpp"
using namespace fast;

cl::Image3D* OpenCLImageAccess3D::get() const {
    return mImage;
}

OpenCLImageAccess3D::OpenCLImageAccess3D(cl::Image3D * image, bool* accessFlag) {
    // Copy the image
    mImage = new cl::Image3D(*image);
    mIsDeleted = false;
    mAccessFlag = accessFlag;
}

void OpenCLImageAccess3D::release() {
    *mAccessFlag = false;
    if(!mIsDeleted) {
        delete mImage;
        mImage = new cl::Image3D(); // assign a new blank object
        mIsDeleted = true;
    }
}

OpenCLImageAccess3D::~OpenCLImageAccess3D() {
    if(!mIsDeleted)
        release();
}
