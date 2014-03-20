#include "Image2D.hpp"
using namespace fast;

Image2D::Image2D(
        PipelineObject * parent,
        oul::Context context,
        cl::Image2D clImage,
        unsigned int width,
        unsigned int height,
        DataType type) {
    mParentPipelineObjects.push_back(parent);
    mCLImages.push_back(clImage);
    mCLContexes.push_back(context);
    mWidth = width;
    mHeight = height;
    mType = type;
}
