#include "ImageAccess.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageAccess::ImageAccess(void* data, Image::pointer image, bool* accessFlag, bool* accessFlag2) {
    mData = data;
    mImage = image;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

void ImageAccess::release() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
}

ImageAccess::~ImageAccess() {
	release();
}

// TODO this should return a copy
void* ImageAccess::get() {
    return mData;
}

template <typename T>
float getScalarAsFloat(T* data, Vector3i position, Vector3i size, uchar channel, uchar nrOfChannels) {

    if(position.x() < 0 || position.y() < 0 || position.z() < 0 ||
            position.x() > size.x()-1 || position.y() > size.y()-1 || position.z() > size.z()-1 || channel >= nrOfChannels)
        throw OutOfBoundsException();

    return data[(position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*nrOfChannels + channel];

}

template <typename T>
void setScalarAsFloat(T* data, Vector3i position, Vector3i size, float value, uchar channel, uchar nrOfChannels) {

    if(position.x() < 0 || position.y() < 0 || position.z() < 0 ||
            position.x() > size.x()-1 || position.y() > size.y()-1 || position.z() > size.z()-1 || channel >= nrOfChannels)
        throw OutOfBoundsException();

    data[(position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*nrOfChannels + channel] = value;

}

float ImageAccess::getScalar(Vector2i position, uchar channel) const {
    Vector3i position3D(position.x(), position.y(), 0);
    return getScalar(position3D, channel);
}

float ImageAccess::getScalar(Vector3i position, uchar channel) const {
    Vector3i size(mImage->getWidth(), mImage->getHeight(), mImage->getDepth());
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(return getScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, size, channel, mImage->getNrOfComponents()))
    }
}

void ImageAccess::setScalar(Vector2i position, float value, uchar channel) {
    Vector3i position3D(position.x(), position.y(), 0);
    setScalar(position3D, value, channel);
}

void ImageAccess::setScalar(Vector3i position, float value, uchar channel) {
    Vector3i size(mImage->getWidth(), mImage->getHeight(), mImage->getDepth());
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(setScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, size, value, channel, mImage->getNrOfComponents()))
    }
}

}
