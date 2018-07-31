#include "ImageToImageNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"

namespace fast {

ImageToImageNetwork::ImageToImageNetwork() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    enableRuntimeMeasurements();
}

void ImageToImageNetwork::execute() {

    /*
    NeuralNetwork::execute();

    tensorflow::Tensor tensor = getNetworkOutput();
    Eigen::Tensor<float, 4, 1> tensor_mapped = tensor.tensor<float, 4>();
    int outputHeight = tensor_mapped.dimension(1);
    int outputWidth = tensor_mapped.dimension(2);

    getAllRuntimes()->printAll();

    Image::pointer output = Image::New();
    auto data = make_uninitialized_unique<char[]>(outputWidth * outputHeight);
    for (int x = 0; x < outputWidth; ++x) {
        for (int y = 0; y < outputHeight; ++y) {
            data[x + y * outputWidth] = (uchar)((tensor_mapped(0, y, x, 0)+1)*127);
        }
    }
    output->create(outputWidth, outputHeight, TYPE_UINT8, 1, (void*)data.get());

    ImageResizer::pointer resizer = ImageResizer::New();
    resizer->setInputData(output);
    resizer->setWidth(mImages.back()->getWidth());
    resizer->setHeight(mImages.back()->getHeight());
    resizer->setPreserveAspectRatio(mPreserveAspectRatio);
    DataPort::pointer port = resizer->getOutputPort();
    resizer->update(0);

    Image::pointer resizedOutput = port->getNextFrame<Image>();
    resizedOutput->setSpacing(mImages.back()->getSpacing());
    addOutputData(0, resizedOutput);
     */
}

}