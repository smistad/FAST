#include "IntensityClipping.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

IntensityClipping::IntensityClipping(float minValue, float maxValue) {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");
    setMinValue(minValue);
    setMaxValue(maxValue);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/IntensityClipping/IntensityClipping.cl");
}

void IntensityClipping::setMinValue(float value) {
    m_min = value;
    setModified(true);
}

void IntensityClipping::setMaxValue(float value) {
    m_max = value;
    setModified(true);
}

void IntensityClipping::execute() {
    auto input = getInputData<Image>();
    if(input->getDimensions() == 3)
        throw Exception("Intensity clipping only supports 2D for now");
    auto output = Image::createFromImage(input);
    output->setSpacing(input->getSpacing());
    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    auto queue = device->getCommandQueue();

    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    cl::Kernel kernel(getOpenCLProgram(device), "clip2D");
    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, m_min);
    kernel.setArg(3, m_max);

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );

    addOutputData(0, output);
}

}