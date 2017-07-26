#include "ImageCropper.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {


void ImageCropper::setOffset(VectorXi offset) {
    mOffset.x() = offset.x();
    mOffset.y() = offset.y();
    if(offset.size() > 2) {
        mOffset.z() = offset.z();
    } else {
        mOffset.z() = 0;
    }
}

void ImageCropper::setSize(VectorXi size) {
    mSize.x() = size.x();
    mSize.y() = size.y();
    if(size.size() > 2) {
        mSize.z() = size.z();
    } else {
        mSize.z() = 1;
    }
}

ImageCropper::ImageCropper() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    mSize = Vector3i::Zero();
    mOffset = Vector3i::Zero();
    mAllowOutOfBoundsCropping = false;
}

void ImageCropper::execute() {
    if(mSize == Vector3i::Zero())
        throw Exception("Size must be given to ImageCropper");

    Image::pointer input = getInputData<Image>();
    Image::pointer output = input->crop(mOffset, mSize, mAllowOutOfBoundsCropping);
    addOutputData(0, output);
}

void ImageCropper::allowOutOfBoundsCropping(bool allow) {
	mAllowOutOfBoundsCropping = allow;
}
}
