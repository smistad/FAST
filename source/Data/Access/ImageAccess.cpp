#include "ImageAccess.hpp"
using namespace fast;

ImageAccess::ImageAccess(void* data, bool* accessFlag, bool* accessFlag2) {
    mData = data;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

void ImageAccess::release() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
}

ImageAccess::~ImageAccess() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
}

void* ImageAccess::get() {
    return mData;
}
