#include "ImageMultiply.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ImageMultiply::ImageMultiply() {
    createInputPort<Image>(0);
    createInputPort<Image>(1);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ImageMultiply/ImageMultiply.cl");
}

void ImageMultiply::execute() {
    Image::pointer input1 = getInputData<Image>(0);
    Image::pointer input2 = getInputData<Image>(1);
    Image::pointer output = getOutputData<Image>();

    if(input1->getDimensions() == 2)
        throw NotImplementedException(__LINE__, __FILE__);

    if(input1->getSize() != input2->getSize())
        throw Exception("Size of both input images to ImageMultiply must be equal", __LINE__, __FILE__);

    output->createFromImage(input1);
    SceneGraph::setParentNode(output, input1);
    Vector3ui size = input1->getSize();

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();

    std::string buildOptions = "-DDATA_TYPE=" + getCTypeAsString(output->getDataType());
    cl::Program program = getOpenCLProgram(device, "", buildOptions);
    cl::Kernel kernel(program, "multiply3D");

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
}

}