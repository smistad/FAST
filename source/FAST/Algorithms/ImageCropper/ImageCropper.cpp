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

void ImageCropper::init() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    mSize = Vector3i::Zero();
    mOffset = Vector3i::Zero();
    mAllowOutOfBoundsCropping = false;
    mCropBottom = 0;
    mCropTop = 0;
}

ImageCropper::ImageCropper(VectorXi size, VectorXi offset) {
    init();
    setOffset(offset);
    setSize(size);
}

ImageCropper::ImageCropper(float cropBottom, float cropTop) {
    init();
    if(cropBottom > 0)
        setCropBottom(cropBottom);
    if(cropTop > 0)
        setCropTop(cropTop);
}

void ImageCropper::execute() {
    Image::pointer input = getInputData<Image>();
    Vector3i size = mSize;
    Vector3i offset = mOffset;
    if(size == Vector3i::Zero()) {
        if(mCropTop > 0) {
            size = input->getSize().cast<int>();
            if(input->getDimensions() == 2) {
                size.y() = (int) std::floor(size.y() * mCropTop);
            } else {
                size.z() = (int) std::floor(size.z() * mCropTop);
            }
        } else if(mCropBottom > 0) {
            size = input->getSize().cast<int>();
            if(input->getDimensions() == 2) {
                offset.y() = (int) std::floor(size.y() * (1.0f - mCropBottom));
                size.y() = (int) std::floor(size.y() * mCropBottom);
            } else {
                offset.z() = (int) std::floor(size.z() * (1.0f - mCropBottom));
                size.z() = (int) std::floor(size.z() * mCropBottom);
            }
        } else {
            throw Exception("ImageCropper need at least non-zero size or cropBottom/Top fraction");
        }
    }

    Image::pointer output = input->crop(offset, size, mAllowOutOfBoundsCropping);
    addOutputData(0, output);
}

void ImageCropper::allowOutOfBoundsCropping(bool allow) {
	mAllowOutOfBoundsCropping = allow;
}

void ImageCropper::setCropBottom(float fraction) {
    mCropBottom = fraction;
}

void ImageCropper::setCropTop(float fraction) {
    mCropTop = fraction;
}

}
