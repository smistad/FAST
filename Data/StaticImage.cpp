#include "StaticImage.hpp"
using namespace fast;

unsigned int StaticImage::getWidth() const {
    return mWidth;
}

unsigned int StaticImage::getHeight() const {
    return mHeight;
}

unsigned char StaticImage::getDimensions() const {
    return mDimensions;
}

DataType StaticImage::getDataType() const {
    return mType;
}

unsigned int StaticImage::getNrOfComponents() const {
    return mComponents;
}
