#include "ImageData.hpp"
using namespace fast;

unsigned int ImageData::getWidth() const {
    return mWidth;
}

unsigned int ImageData::getHeight() const {
    return mHeight;
}

unsigned char ImageData::getDimensions() const {
    return mDimensions;
}
