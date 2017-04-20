#include "ImageAccess.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageAccess::ImageAccess(void* data, Image::pointer image) {
    mData = data;
    mImage = image;
}

void ImageAccess::release() {
	mImage->accessFinished();
}

ImageAccess::~ImageAccess() {
	release();
}

// TODO this should return a copy
void* ImageAccess::get() {
    return mData;
}

template <typename T>
float getScalarAsFloat(T* data, VectorXi position, Image::pointer image, uchar channel) {

    Vector3ui size = image->getSize();
    if(position.x() < 0 || position.y() < 0 || position.z() < 0 ||
            position.x() > size.x()-1 || position.y() > size.y()-1 || position.z() > size.z()-1 || channel >= image->getNrOfComponents())
        throw OutOfBoundsException();

    T value = data[(position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*image->getNrOfComponents() + channel];
    float floatValue;
    if(image->getDataType() == TYPE_SNORM_INT16) {
        floatValue = std::max(-1.0f, (float)value / 32767.0f);
    } else if(image->getDataType() == TYPE_UNORM_INT16) {
        floatValue = (float)value / 65535.0f;
    } else {
        floatValue = value;
    }

    return floatValue;
}

template <typename T>
float getScalarAsFloat(T* data, uint position, Image::pointer image, uchar channel) {

    Vector3ui size = image->getSize();
    if(position >= size.x()*size.y()*size.z())
        throw OutOfBoundsException();

    T value = data[position*image->getNrOfComponents() + channel];
    float floatValue;
    if(image->getDataType() == TYPE_SNORM_INT16) {
        floatValue = std::max(-1.0f, (float)value / 32767.0f);
    } else if(image->getDataType() == TYPE_UNORM_INT16) {
        floatValue = (float)value / 65535.0f;
    } else {
        floatValue = value;
    }

    return floatValue;
}

template <typename T>
void setScalarAsFloat(T* data, VectorXi position, Image::pointer image, float value, uchar channel) {

    Vector3ui size = image->getSize();
    if(position.x() < 0 || position.y() < 0 || position.z() < 0 ||
            position.x() > size.x()-1 || position.y() > size.y()-1 || position.z() > size.z()-1 || channel >= image->getNrOfComponents())
        throw OutOfBoundsException();

    uint address = (position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*image->getNrOfComponents() + channel;
    if(image->getDataType() == TYPE_SNORM_INT16) {
        data[address] = value * 32767.0f;;
    } else if(image->getDataType() == TYPE_UNORM_INT16) {
        data[address] = value * 65535.0f;;
    } else {
        data[address] = value;
    }
}

template <typename T>
void setScalarAsFloat(T* data, uint position, Image::pointer image, float value, uchar channel) {

    Vector3ui size = image->getSize();
    if(position >= size.x()*size.y()*size.z())
        throw OutOfBoundsException();

    uint address = position*image->getNrOfComponents() + channel;
    if(image->getDataType() == TYPE_SNORM_INT16) {
        data[address] = value * 32767.0f;;
    } else if(image->getDataType() == TYPE_UNORM_INT16) {
        data[address] = value * 65535.0f;;
    } else {
        data[address] = value;
    }
}

float ImageAccess::getScalar(VectorXi position, uchar channel) const {
    if(mImage->getDimensions() == 2)
        position = Vector3i(position.x(), position.y(), 0);
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(return getScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, mImage, channel))
    }
}

float ImageAccess::getScalar(uint position, uchar channel) const {
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(return getScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, mImage, channel))
    }
}

void ImageAccess::setScalar(VectorXi position, float value, uchar channel) {
    Vector3i size(mImage->getWidth(), mImage->getHeight(), mImage->getDepth());
    if(mImage->getDimensions() == 2)
        position = Vector3i(position.x(), position.y(), 0);
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(setScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, mImage, value, channel))
    }
}

void ImageAccess::setScalar(uint position, float value, uchar channel) {
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(setScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, mImage, value, channel))
    }
}

Vector4f ImageAccess::getVector(VectorXi position) const {
    Vector4f result;
    for(uchar i = 0; i < mImage->getNrOfComponents(); ++i) {
        result[i] = getScalar(position, i);
    }
    return result;
}

void ImageAccess::setVector(VectorXi position, Vector4f value) {
    for(uchar i = 0; i < mImage->getNrOfComponents(); ++i) {
        setScalar(position, value[i], i);
    }
}

void ImageAccess::setVector(uint position, Vector4f value) {
    for(uchar i = 0; i < mImage->getNrOfComponents(); ++i) {
        setScalar(position, value[i], i);
    }
}

} // end namespace fast
