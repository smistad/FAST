#include "Segmentation.hpp"

namespace fast {

void Segmentation::createFromImage(Image::pointer image,
        ExecutionDevice::pointer device) {
    if(image->getDimensions() == 2) {
        create2DImage(image->getWidth(), image->getHeight(), TYPE_UINT8, 1, device);
    } else {
        create3DImage(image->getWidth(), image->getHeight(), image->getDepth(), TYPE_UINT8, 1, device);
    }

    SceneGraph::setParentNode(mPtr.lock(), image);
    setSpacing(image->getSpacing());
}

Segmentation::Segmentation() {
}

Segmentation::~Segmentation() {
    freeAll();
}

}
