#include "Image2D.hpp"
using namespace fast;



void Image2D::setOpenCLImage(cl::Image2D *clImage, OpenCLDevice::Ptr device) {
    mCLImages.push_back(clImage);
    mCLDevices.push_back(device);
    mWidth = clImage->getImageInfo<CL_IMAGE_WIDTH>();
    mHeight = clImage->getImageInfo<CL_IMAGE_HEIGHT>();
}

OpenCLImageAccess2D Image2D::getOpenCLImageAccess(
        accessType type,
        OpenCLDevice::Ptr device) {
    // TODO: Check for write access

    // Check to see if image exist on the device
    for(int i = 0; i < mCLDevices.size(); i++) {
        if(mCLDevices[i] == device) {
            return OpenCLImageAccess2D(mCLImages[i]);
        }
    }

    // TODO: Transfer data to a new device
}

Image2D::~Image2D() {
    // Delete all images
    for(int i = 0; i < mCLImages.size(); i++) {
        delete mCLImages[i];
    }
}
