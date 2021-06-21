#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/Image.hpp>
#include "TensorToImage.hpp"

namespace fast {

TensorToImage::TensorToImage() {
    createInputPort<Tensor>(0);
    createOutputPort<Image>(0);
}

void TensorToImage::execute() {
    auto tensor = getInputData<Tensor>();
    const auto shape = tensor->getShape();
    auto access = tensor->getAccess(ACCESS_READ);
    const int dims = shape.getDimensions();
    const int channels = shape[0];
    const int outputHeight = shape[dims-3];
    const int outputWidth = shape[dims-2];
    int outputDepth = 1;
    float* tensorData = access->getRawData();
    if(dims == 5) {
        outputDepth = shape[dims - 4];
    }

    Image::pointer image;
    if(outputDepth == 1) {
        image = Image::create(outputWidth, outputHeight, TYPE_FLOAT, channels, std::move(tensorData));
    } else {
        image = Image::create(outputWidth, outputHeight, outputDepth, TYPE_FLOAT, channels, std::move(tensorData));
    }
    image->setSpacing(tensor->getSpacing());
    SceneGraph::setParentNode(image, tensor);
    addOutputData(0, image);
}

}