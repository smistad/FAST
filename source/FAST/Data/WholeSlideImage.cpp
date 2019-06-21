#include "WholeSlideImage.hpp"

namespace fast {

void WholeSlideImage::create(std::vector<WholeSlideImageLevel> levels) {
    m_levels = levels;
    int width = levels.back().width;
    int height = levels.back().height;
    mBoundingBox = BoundingBox(Vector3f(width, height, 0));
}

void WholeSlideImage::free(ExecutionDevice::pointer device) {
    freeAll();
}

void WholeSlideImage::freeAll() {
    m_levels.clear();
}

}