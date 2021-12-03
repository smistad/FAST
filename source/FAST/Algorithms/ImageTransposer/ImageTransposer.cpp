#include "ImageTransposer.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImageTransposer::ImageTransposer(std::vector<int> axes) {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageTransposer/ImageTransposer2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageTransposer/ImageTransposer3D.cl", "3D");
    createIntegerAttribute("axes", "Axes", "Axes", 0);
    setAxes(axes);
}

void ImageTransposer::execute() {
    auto input = getInputData<Image>();
    Image::pointer output;
    if(input->getDimensions() == 3) {
        // 3D
        auto axes = m_axes;
        if(axes.empty())
            axes = {1, 0, 2};
        Vector3ui size = input->getSize();
        Vector3f spacing = input->getSpacing();
        output = Image::create(size[axes[0]], size[axes[1]], size[axes[2]], input->getDataType(), input->getNrOfChannels());
        output->setSpacing(spacing[axes[0]], spacing[axes[1]], spacing[axes[2]]);

        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);


        auto program = getOpenCLProgram(device, "3D", "-DTYPE=" + getCTypeAsString(input->getDataType()));
        cl::Kernel kernel(program, "transpose3D");

        kernel.setArg(0, *inputAccess->get3DImage());
        kernel.setArg(1, *outputAccess->get());
        kernel.setArg(2, axes[0]);
        kernel.setArg(3, axes[1]);
        kernel.setArg(4, axes[2]);
        kernel.setArg(5, input->getNrOfChannels());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
                );
    } else {
        // 2D
        const auto d1 = input->getWidth();
        const auto d2 = input->getHeight();
        output = Image::create(d2, d1, input->getDataType(), input->getNrOfChannels());
        output->setSpacing(input->getSpacing().y(), input->getSpacing().x(), input->getSpacing().z());

        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        auto program = getOpenCLProgram(device, "2D");
        cl::Kernel kernel(program, "transpose2D");

        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
                );
    }

    addOutputData(0, output);
}

void ImageTransposer::setAxes(std::vector<int> axes) {
    if(axes.empty())
        return;
    if(axes.size() != 3)
        throw Exception("Axes vector given to ImageTransposer must have length 3");
    bool foundZero = false;
    bool foundOne = false;
    bool foundTwo = false;
    for(int i = 0; i < 3; ++i) {
        if(axes[i] == 0) {
            if(foundZero)
                throw Exception("Axes vector given to ImageTransposer must contain the values 0, 1, and 2");
            foundZero = true;
        } else if(axes[i] == 1) {
            if(foundOne)
                throw Exception("Axes vector given to ImageTransposer must contain the values 0, 1, and 2");
            foundOne = true;
        } else if(axes[i] == 2) {
            if(foundTwo)
                throw Exception("Axes vector given to ImageTransposer must contain the values 0, 1, and 2");
            foundTwo = true;
        }
    }
    if(!foundZero || !foundOne || !foundTwo)
        throw Exception("Axes vector given to ImageTransposer must contain the values 0, 1, and 2");
    m_axes = axes;
}

void ImageTransposer::loadAttributes() {
    setAxes(getIntegerListAttribute("axes"));
}


}