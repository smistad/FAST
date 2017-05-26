#include "Dilation.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

Dilation::Dilation() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Morphology/Dilation.cl");
    mSize = 3;
}

void Dilation::setStructuringElementSize(int size) {
    if(size % 2 == 0) {
        throw Exception("Structuring element size given to Dilation must be odd");
    }
    if(size <= 1) {
        throw Exception("Structuring element size given to Dilation must be > 2");
    }
    mSize = size;
}

void Dilation::execute() {
    Image::pointer input = getStaticInputData<Image>();
    if(input->getDataType() != TYPE_UINT8) {
        throw Exception("Data type of image given to Dilation must be UINT8");
    }

    Image::pointer output = getStaticOutputData<Image>();
    output->createFromImage(input);
    output->fill(0);

    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel dilateKernel(program, "dilate");

    Vector3ui size = input->getSize();

    OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
    dilateKernel.setArg(0, *access->get3DImage());
    dilateKernel.setArg(2, mSize/2);

    if(!device->isWritingTo3DTexturesSupported()) {
        OpenCLBufferAccess::pointer access2 = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        dilateKernel.setArg(1, *access2->get());

        queue.enqueueNDRangeKernel(
            dilateKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );
    } else {
        OpenCLImageAccess::pointer access2 = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        dilateKernel.setArg(1, *access2->get3DImage());

        queue.enqueueNDRangeKernel(
            dilateKernel,
            cl::NullRange,
            cl::NDRange(size.x(), size.y(), size.z()),
            cl::NullRange
        );
    }

}

}