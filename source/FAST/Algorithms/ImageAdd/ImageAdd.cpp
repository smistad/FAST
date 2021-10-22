#include "ImageAdd.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageAdd::ImageAdd() {
    createInputPort(0);
    createInputPort(1);
    createOutputPort(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageAdd/ImageAdd.cl");
}

void ImageAdd::execute() {
    auto input1 = getInputData<Image>(0);
    auto input2 = getInputData<Image>(1);

    if(input1->getSize() != input2->getSize())
        throw Exception("Size of both input images to ImageAdd must be equal");

    auto output = Image::createFromImage(input1);
    SceneGraph::setParentNode(output, input1);
    Vector3ui size = input1->getSize();

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();


    std::string buildOptions = "-DDATA_TYPE=" + getCTypeAsString(output->getDataType());
    if(input1->getDimensions() == 3) {
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel(program, "add3D");

        OpenCLImageAccess::pointer access1 = input1->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer access2 = input2->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLBufferAccess::pointer access3 = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *access1->get3DImage());
        kernel.setArg(1, *access2->get3DImage());
        kernel.setArg(2, *access3->get());
        kernel.setArg(3, output->getNrOfChannels());

        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(size.x(), size.y(), size.z()),
                cl::NullRange
        );
    } else {
        cl::Program program = getOpenCLProgram(device, "", buildOptions);
        cl::Kernel kernel(program, "add2D");

        auto access1 = input1->getOpenCLImageAccess(ACCESS_READ, device);
        auto access2 = input2->getOpenCLImageAccess(ACCESS_READ, device);
        auto access3 = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *access1->get2DImage());
        kernel.setArg(1, *access2->get2DImage());
        kernel.setArg(2, *access3->get2DImage());
        kernel.setArg(3, input1->getNrOfChannels());
        kernel.setArg(4, input2->getNrOfChannels());
        kernel.setArg(5, output->getNrOfChannels());

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