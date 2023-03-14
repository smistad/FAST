#include "IntensityClipping.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

void IntensityClipping::loadAttributes() {
    setMinValue(getFloatAttribute("min"));
    setMinValue(getFloatAttribute("max"));
}

IntensityClipping::IntensityClipping(float minValue, float maxValue) {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/IntensityClipping/IntensityClipping2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/IntensityClipping/IntensityClipping3D.cl", "3D");
    createFloatAttribute("min", "Minimum intensity", "Minimum intensity", minValue);
    createFloatAttribute("max", "Maximum intensity", "Maximum intensity", maxValue);
    setMinValue(minValue);
    setMaxValue(maxValue);
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
    auto output = Image::createFromImage(input);
    output->setSpacing(input->getSpacing());
    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    auto queue = device->getCommandQueue();

    if (input->getDimensions() == 2) {
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Kernel kernel(getOpenCLProgram(device, "2D"), "clip2D");
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
    }
    else {
        // 3D
        if (input->getNrOfChannels() > 1)
            throw Exception("Support for multi-channel 3D is not implemented in IntensityClipping");
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        auto program = getOpenCLProgram(device, "3D", "-DTYPE=" + getCTypeAsString(input->getDataType()));
		cl::Kernel kernel(program, "clip3D");
        kernel.setArg(0, *inputAccess->get3DImage());
        kernel.setArg(1, *outputAccess->get());
        kernel.setArg(2, m_min);
        kernel.setArg(3, m_max);

        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
            cl::NullRange
        );

    }

    addOutputData(0, output);
}

}
