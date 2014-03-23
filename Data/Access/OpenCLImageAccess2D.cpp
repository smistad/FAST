#include "OpenCLImageAccess2D.hpp"
using namespace fast;

cl::Image2D* OpenCLImageAccess2D::get() const {
    return mImage;
}

OpenCLImageAccess2D::OpenCLImageAccess2D(cl::Image2D * image) {
    // Copy the image
    mImage = new cl::Image2D(*image);
    mIsDeleted = false;
}

void OpenCLImageAccess2D::release() {
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
