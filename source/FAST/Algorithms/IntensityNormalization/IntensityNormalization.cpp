#include "IntensityNormalization.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

IntensityNormalization::IntensityNormalization(float valueLow, float valueHigh, float minIntensity, float maxIntensity) {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/IntensityNormalization/IntensityNormalization.cl");
    mLow = valueLow;
    mHigh = valueHigh;
    m_minIntensity = minIntensity;
    m_maxIntensity = maxIntensity;
}

void IntensityNormalization::setLowestValue(float value) {
    mLow = value;
    setModified(true);
}

void IntensityNormalization::setHighestValue(float value) {
    mHigh = value;
    setModified(true);
}

void IntensityNormalization::execute() {
    if(mHigh <= mLow)
        throw Exception("The high value must be higher than the low value in IntensityNormalization.");

    auto input = getInputData<Image>();

    const uint width = input->getWidth();
    const uint height = input->getHeight();
    const uint depth = input->getDepth();
    cl::NDRange globalSize;

    float minimum = m_minIntensity;
    if(std::isnan(minimum)) {
        minimum = input->calculateMinimumIntensity();
    }
    float maximum = m_maxIntensity;
    if(std::isnan(maximum)) {
        maximum = input->calculateMaximumIntensity();
    }
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program = getOpenCLProgram(device);
    cl::Kernel kernel;

    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    Image::pointer output;
    if(input->getDimensions() == 2) {
        output = Image::create(width, height, TYPE_FLOAT, input->getNrOfChannels());
        globalSize = cl::NDRange(width, height);
        OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel = cl::Kernel(program, "scaleImage2D");
        kernel.setArg(0, *(inputAccess->get2DImage()));
        kernel.setArg(1, *(outputAccess->get2DImage()));
    } else {
        output = Image::create(width, height, depth, TYPE_FLOAT, input->getNrOfChannels());
        globalSize = cl::NDRange(width, height, depth);
        kernel = cl::Kernel(program, "scaleImage3D");
        kernel.setArg(0, *(inputAccess->get3DImage()));
        if(device->isWritingTo3DTexturesSupported()) {
            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(1, *(outputAccess->get3DImage()));
        } else {
            OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
            kernel.setArg(1, *(outputAccess->get()));
            kernel.setArg(6, output->getNrOfChannels());
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

    addOutputData(0, output);
}

void IntensityNormalization::setMinimumIntensity(float intensity) {
    m_minIntensity = intensity;
    setModified(true);
}

void IntensityNormalization::setMaximumIntensity(float intensity) {
    m_maxIntensity = intensity;
    setModified(true);
}

} // end namespace fast


