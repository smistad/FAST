#include "Image2D.hpp"
using namespace fast;

void Image2D::addParent(PipelineObject::Ptr parent) {
    mParentPipelineObjects.push_back(parent);
}

void Image2D::setOpenCLImage(cl::Image2D clImage, oul::Context context) {
    mCLImages.push_back(clImage);
    mCLContexes.push_back(context);
    mWidth = clImage.getImageInfo<CL_IMAGE_WIDTH>();
    mHeight = clImage.getImageInfo<CL_IMAGE_HEIGHT>();
}
