#include "ImageInverter.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Utility.hpp"

namespace fast {

ImageInverter::ImageInverter() {
    createInputPort(0);
    createOutputPort(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageInverter/ImageInverter2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageInverter/ImageInverter3D.cl", "3D");
}

void ImageInverter::execute() {
    auto input = getInputData<Image>();

    float max = input->calculateMaximumIntensity();
    float min = input->calculateMinimumIntensity();

    auto output = Image::createFromImage(input);
    Vector3ui size = input->getSize();

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();

    if(input->getDimensions() == 3) {
        std::string buildOptions = "-DDATA_TYPE=" + getCTypeAsString(output->getDataType());
        cl::Program program = getOpenCLProgram(device, "3D", buildOptions);
        cl::Kernel kernel(program, "invert3D");

        auto access = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto access2 = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
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
    } else {
        cl::Program program = getOpenCLProgram(device, "2D");
        cl::Kernel kernel(program, "invert2D");

        auto access = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto access2 = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *access->get2DImage());
        kernel.setArg(1, *access2->get2DImage());
        kernel.setArg(2, min);
        kernel.setArg(3, max);

        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(size.x(), size.y()),
                cl::NullRange
        );
    }
    addOutputData(0, output);
}

}
