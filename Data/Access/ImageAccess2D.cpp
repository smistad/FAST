#include "Image2D.hpp"
using namespace fast;

ImageAccess2D::ImageAccess2D(void* data) {
    mData = data;
}

void* ImageAccess2D::get() {
    return mData;
}
