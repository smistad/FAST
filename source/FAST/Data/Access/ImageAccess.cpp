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
float getScalarAsFloat(T* data, VectorXi position, Vector3i size, uchar channel, uchar nrOfChannels) {

    if(position.x() < 0 || position.y() < 0 || position.z() < 0 ||
            position.x() > size.x()-1 || position.y() > size.y()-1 || position.z() > size.z()-1 || channel >= nrOfChannels)
        throw OutOfBoundsException();

    return data[(position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*nrOfChannels + channel];

}

template <typename T>
void setScalarAsFloat(T* data, VectorXi position, Vector3i size, float value, uchar channel, uchar nrOfChannels) {

    if(position.x() < 0 || position.y() < 0 || position.z() < 0 ||
            position.x() > size.x()-1 || position.y() > size.y()-1 || position.z() > size.z()-1 || channel >= nrOfChannels)
        throw OutOfBoundsException();

    data[(position.x() + position.y()*size.x() + position.z()*size.x()*size.y())*nrOfChannels + channel] = value;

}

float ImageAccess::getScalar(VectorXi position, uchar channel) const {
    Vector3i size(mImage->getWidth(), mImage->getHeight(), mImage->getDepth());
    if(mImage->getDimensions() == 2)
        position = Vector3i(position.x(), position.y(), 0);
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(return getScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, size, channel, mImage->getNrOfComponents()))
    }
}

void ImageAccess::setScalar(VectorXi position, float value, uchar channel) {
    Vector3i size(mImage->getWidth(), mImage->getHeight(), mImage->getDepth());
    if(mImage->getDimensions() == 2)
        position = Vector3i(position.x(), position.y(), 0);
    switch(mImage->getDataType()) {
        fastSwitchTypeMacro(setScalarAsFloat<FAST_TYPE>((FAST_TYPE*)mData, position, size, value, channel, mImage->getNrOfComponents()))
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

} // end namespace fast
