#include "ScaleImage.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

ScaleImage::ScaleImage() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/ScaleImage/ScaleImage.cl");
    mLow = 0.0f;
    mHigh = 1.0f;
}

void ScaleImage::setLowestValue(float value) {
    mLow = value;
}

void ScaleImage::setHighestValue(float value) {
    mHigh = value;
}

void ScaleImage::execute() {
    if(mHigh <= mLow)
        throw Exception("The high value must be higher than the low value in ScaleImage.");

    Image::pointer input = getStaticInputData<Image>();
    Image::pointer output = getStaticOutputData<Image>();

    const uint width = input->getWidth();
    const uint height = input->getHeight();
    const uint depth = input->getDepth();
    cl::NDRange globalSize;

    float minimum = input->calculateMinimumIntensity();
    float maximum = input->calculateMaximumIntensity();

    OpenCLDevice::pointer device = getMainDevice();
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel;

    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    if(input->getDimensions() == 2) {
        output->create(width, height, TYPE_FLOAT, input->getNrOfComponents());
        globalSize = cl::NDRange(width, height);
        OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel = cl::Kernel(program, "scaleImage2D");
        kernel.setArg(0, *(inputAccess->get2DImage()));
        kernel.setArg(1, *(outputAccess->get2DImage()));
    } else {
        output->create(width, height, depth, TYPE_FLOAT, input->getNrOfComponents());
        globalSize = cl::NDRange(width, height, depth);
        kernel = cl::Kernel(program, "scaleImage3D");
        kernel.setArg(0, *(inputAccess->get3DImage()));
        if(device->isWritingTo3DTexturesSupported()) {
            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(1, *(outputAccess->get3DImage()));
        } else {
            OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(1, *(outputAccess->get()));
            kernel.setArg(6, output->getNrOfComponents());
        }
    }
    output->setSpacing(input->getSpacing());
    SceneGraph::setParentNode(output, input);

    kernel.setArg(2, minimum);
    kernel.setArg(3, maximum);
    kernel.setArg(4, mLow);
    kernel.setArg(5, mHigh);

    cl::CommandQueue queue = device->getCommandQueue();

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            globalSize,
            cl::NullRange
    );
}

} // end namespace fast


