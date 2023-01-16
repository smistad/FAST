#include "GrayscaleToColor.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

GrayscaleToColor::GrayscaleToColor(bool addAlphaChannel) {
    createInputPort(0);
    createOutputPort(0);
    m_addAlphaChannel = addAlphaChannel;
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Color/GrayscaleToColor.cl");
}

void GrayscaleToColor::execute() {
    auto image = getInputData<Image>();
    if(image->getDimensions() != 2)
        throw Exception("GrayscaleToColor is only implemented for 2D");

    if(image->getNrOfChannels() >= 2) {
        // Image is already color..
        addOutputData(image);
        return;
    }

    auto output = Image::create(image->getSize(), image->getDataType(), m_addAlphaChannel ? 4 : 3);
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