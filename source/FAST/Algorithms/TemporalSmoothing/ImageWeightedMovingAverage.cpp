#include "ImageWeightedMovingAverage.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImageWeightedMovingAverage::ImageWeightedMovingAverage(int frameCount, bool keepDataType) {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/TemporalSmoothing/ImageWeightedMovingAverage.cl");

    setFrameCount(frameCount);
    setKeepDataType(keepDataType);
    createIntegerAttribute("frame-count", "Frame count", "Nr of frames to use in moving average", m_frameCount);
    createBooleanAttribute("keep-datatype", "Keep data type", "Whether to keep data type of input image for output image, or use float instead", m_keepDataType);
}

void ImageWeightedMovingAverage::reset() {
    m_buffer.clear();
    m_memory.reset();
}

void ImageWeightedMovingAverage::setFrameCount(int frameCount) {
    if(frameCount <= 0)
        throw Exception("Frame count must be > 0");
    m_frameCount = frameCount;
    reset();
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
    m_buffer.push_back(input);

    if(input->getDimensions() != 2)
        throw Exception("ImageWeightedMovingAverage only supports 2D atm");
    if (input->getNrOfChannels() > 1)
        reportWarning() << "ImageWeightedMovingAverage only supports single channel images" << reportEnd();

    if(!m_memory) {
        m_memory = Image::create(input->getSize(), TYPE_FLOAT, 2);
        m_memory->fill(0);
    }
    if(input->getSize() != m_memory->getSize())
        throw Exception("Image input to ImageWeightedMovingAverage suddenly changed size.");

    auto memoryOut = Image::createFromImage(m_memory);

    Image::pointer output;
    if(m_keepDataType) {
        output = Image::create(input->getSize(), input->getDataType(), 1);
    } else {
        output = Image::create(input->getSize(), TYPE_FLOAT, 1);
    }
    SceneGraph::setParentNode(output, input);
    output->setSpacing(input->getSpacing());

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto program = getOpenCLProgram(device);

    if(m_buffer.size() <= m_frameCount) {
        while(m_buffer.size() <= m_frameCount) // Initialization, fill up buffer
            m_buffer.push_back(input);

        // Run initialization kernel
        cl::Kernel kernel(program, "WMAinitialize");
        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        auto memoryAccess = m_memory->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, *memoryAccess->get2DImage());
        kernel.setArg(3, m_frameCount);

        device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
        );

        m_buffer.pop_front();
        addOutputData(0, output);

        return;
    }
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
    kernel.setArg(5, m_frameCount);

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );

    m_buffer.pop_front();
    m_memory = memoryOut;
    addOutputData(0, output);
}

}
