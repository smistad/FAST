#include "Image.hpp"
using namespace fast;

ImageAccess2D::ImageAccess2D(void* data, bool* accessFlag) {
    mData = data;
    mAccessFlag = accessFlag;
}

ImageAccess2D::~ImageAccess2D() {
    *mAccessFlag = false;
}

void* ImageAccess2D::get() {
    return mData;
}
