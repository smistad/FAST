#include "VectorMedianFilter.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {


VectorMedianFilter::VectorMedianFilter(int size) {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/VectorMedianFilter/VectorMedianFilter.cl");

    setWindowSize(size);
    createIntegerAttribute("window-size", "Window size", "Size of area to perform median filter on", m_windowSize);
}

void VectorMedianFilter::loadAttributes() {
    setWindowSize(getIntegerAttribute("window-size"));
}

void VectorMedianFilter::execute() {
    auto input = getInputData<Image>(0);
    auto output = Image::createFromImage(input);

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto queue = device->getCommandQueue();
    cl::Kernel kernel(getOpenCLProgram(device), "vectorMedianFilter");
    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, (int)(m_windowSize-1)/2);

    queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,
        cl::NDRange(input->getWidth(), input->getHeight()),
        cl::NullRange
    );
    addOutputData(0, output);
}

void VectorMedianFilter::setWindowSize(int size) {
    if(size < 3 || size % 2 == 0)
        throw Exception("Window size must be >= 3 and odd");
    m_windowSize = size;
}


}