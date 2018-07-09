#include "ImageInverter.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Utility.hpp"

namespace fast {

ImageInverter::ImageInverter() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageInverter/ImageInverter.cl");
}

void ImageInverter::execute() {
    Image::pointer input = getInputData<Image>();
    Image::pointer output = getOutputData<Image>();

    float max = input->calculateMaximumIntensity();
    float min = input->calculateMinimumIntensity();

    output->createFromImage(input);
    Vector3ui size = input->getSize();

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();

    std::string buildOptions = "-DDATA_TYPE=" + getCTypeAsString(output->getDataType());
    cl::Program program = getOpenCLProgram(device, "", buildOptions);
    cl::Kernel kernel(program, "invert3D");

    OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLBufferAccess::pointer access2 = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
    kernel.setArg(0, *access->get3DImage());
    kernel.setArg(1, *access2->get());
    kernel.setArg(2, min);
    kernel.setArg(3, max);
    kernel.setArg(4, output->getNrOfChannels());

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
    );

}

}
