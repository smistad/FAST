#include "Erosion.hpp"
#include "FAST/Data/Segmentation.hpp"

namespace fast {

Erosion::Erosion() {
    createInputPort<Segmentation>(0);
    createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Morphology/Erosion.cl");
    mSize = 3;
}

void Erosion::setStructuringElementSize(int size) {
    if(size % 2 == 0) {
        throw Exception("Structuring element size given to Erosion must be odd");
    }
    if(size <= 1) {
        throw Exception("Structuring element size given to Erosion must be > 2");
    }
    mSize = size;
}

void Erosion::execute() {
    Segmentation::pointer input = getStaticInputData<Segmentation>();

    Segmentation::pointer output = getStaticOutputData<Segmentation>();
    output->createFromImage(input);
    output->fill(0);

    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel(program, "erode");

    Vector3ui size = input->getSize();

    OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
    kernel.setArg(0, *access->get3DImage());
    kernel.setArg(2, mSize/2);

    if(!device->isWritingTo3DTexturesSupported()) {
        OpenCLBufferAccess::pointer access2 = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(1, *access2->get());

        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(size.x(), size.y(), size.z()),
                cl::NullRange
        );
    } else {
        OpenCLImageAccess::pointer access2 = input->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(1, *access2->get3DImage());

        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(size.x(), size.y(), size.z()),
                cl::NullRange
        );
    }

}

}
