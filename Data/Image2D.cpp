#include "Image2D.hpp"
using namespace fast;



void Image2D::setOpenCLImage(cl::Image2D clImage, OpenCLDevice::Ptr device) {
    mCLImages.push_back(clImage);
    mCLDevices.push_back(device);
    mWidth = clImage.getImageInfo<CL_IMAGE_WIDTH>();
    mHeight = clImage.getImageInfo<CL_IMAGE_HEIGHT>();
}
