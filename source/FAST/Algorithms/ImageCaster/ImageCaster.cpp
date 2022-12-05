#include "ImageCaster.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImageCaster::ImageCaster() {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImageCaster/ImageCaster.cl");
}

ImageCaster::ImageCaster(DataType outputType, float scaleFactor) : ImageCaster() {
    m_outputType = outputType;
    m_scaleFactor = scaleFactor;
}

void ImageCaster::execute() {
    auto input = getInputData<Image>();
    if(input->getDimensions() == 3)
        throw Exception("Image caster only supports 2D for now");
    auto output = Image::create(input->getSize(), m_outputType, input->getNrOfChannels());
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);
    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    auto queue = device->getCommandQueue();

    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    cl::Kernel kernel(getOpenCLProgram(device), "cast2D");
    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, m_scaleFactor);

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );

    addOutputData(0, output);
}

}