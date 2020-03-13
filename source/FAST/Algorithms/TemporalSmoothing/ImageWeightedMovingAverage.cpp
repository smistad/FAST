#include "ImageWeightedMovingAverage.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImageWeightedMovingAverage::ImageWeightedMovingAverage() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/TemporalSmoothing/ImageWeightedMovingAverage.cl");

    m_frameCount = 10;
    m_keepDataType = false;
    createIntegerAttribute("frame-count", "Frame count", "Nr of frames to use in moving average", m_frameCount);
    createBooleanAttribute("keep-datatype", "Keep data type", "Whether to keep data type of input image for output image, or use float instead", m_keepDataType);
}

void ImageWeightedMovingAverage::setFrameCount(int frameCount) {
    if(frameCount <= 0)
        throw Exception("Frame count must be > 0");
    m_frameCount = frameCount;
}

void ImageWeightedMovingAverage::setKeepDataType(bool keep) {
    m_keepDataType = keep;
}

void ImageWeightedMovingAverage::loadAttributes() {
    setFrameCount(getIntegerAttribute("frame-count"));
    setKeepDataType(getBooleanAttribute("keep-datatype"));
}

void ImageWeightedMovingAverage::execute() {
    auto input = getInputData<Image>(0);
    auto output = getOutputData<Image>(0);
    m_buffer.push(input);

    if(input->getDimensions() != 2)
        throw Exception("ImageWeightedMovingAverage only supports 2D atm");
    if (input->getNrOfChannels() > 1)
        reportWarning() << "ImageWeightedMovingAverage only supports single channel images" << reportEnd();

    if(!m_memory) {
        m_memory = Image::New();
        m_memory->create(input->getSize(), TYPE_FLOAT, 2);
        m_memory->fill(0);
    }
    if(input->getSize() != m_memory->getSize())
        throw Exception("Image input to ImageWeightedMovingAverage suddenly changed size.");

    auto memoryOut = Image::New();
    memoryOut->createFromImage(m_memory);

    if(m_keepDataType) {
        output->create(input->getSize(), input->getDataType(), 1);
    } else {
        output->create(input->getSize(), TYPE_FLOAT, 1);
    }

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto program = getOpenCLProgram(device);

    auto last = m_buffer.front();

    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto lastAccess = last->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    auto memoryAccess = m_memory->getOpenCLImageAccess(ACCESS_READ, device);
    auto memoryOutAccess = memoryOut->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    cl::Kernel kernel(program, "WMAiteration");
    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *memoryAccess->get2DImage());
    kernel.setArg(2, *lastAccess->get2DImage());
    kernel.setArg(3, *outputAccess->get2DImage());
    kernel.setArg(4, *memoryOutAccess->get2DImage());
    kernel.setArg(5, (int)m_buffer.size());
    kernel.setArg(6, (int)m_frameCount);

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );

    if(m_buffer.size() > m_frameCount)
        m_buffer.pop();

    m_memory = memoryOut;
}

}
