#include "ColorToGrayscale.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {
ColorToGrayscale::ColorToGrayscale() {
    createInputPort(0);
    createOutputPort(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Color/ColorToGrayscale.cl");
}

void ColorToGrayscale::execute() {
    auto image = getInputData<Image>();
    if(image->getDimensions() != 2)
        throw Exception("ColorToGrayscale is only implemented for 2D");

    if(image->getNrOfChannels() == 1) {
        // Image is already grayscale..
        addOutputData(image);
        return;
    }

    auto output = Image::create(image->getSize(), image->getDataType(), 1);
    output->setSpacing(image->getSpacing());
    SceneGraph::setParentNode(output, image);

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    cl::Kernel kernel(getOpenCLProgram(device), "convert");

    auto inputAccess = image->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(image->getWidth(), image->getHeight()),
            cl::NullRange
    );

    addOutputData(output);
}
}