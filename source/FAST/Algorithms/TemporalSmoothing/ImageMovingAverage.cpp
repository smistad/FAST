#include "ImageMovingAverage.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ImageMovingAverage::ImageMovingAverage() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/TemporalSmoothing/ImageMovingAverage.cl");

    m_frameCount = 10;
    m_keepDataType = false;
    createIntegerAttribute("frame-count", "Frame count", "Nr of frames to use in moving average", m_frameCount);
    createBooleanAttribute("keep-datatype", "Keep data type", "Whether to keep data type of input image for output image, or use float instead", m_keepDataType);
}

void ImageMovingAverage::reset() {
    m_buffer.clear();
    m_memory.reset();
}

void ImageMovingAverage::setFrameCount(int frameCount) {
    if(frameCount <= 0)
        throw Exception("Frame count must be > 0");
    m_frameCount = frameCount;
    reset();
}

void ImageMovingAverage::setKeepDataType(bool keep) {
    m_keepDataType = keep;
}

void ImageMovingAverage::loadAttributes() {
    setFrameCount(getIntegerAttribute("frame-count"));
    setKeepDataType(getBooleanAttribute("keep-datatype"));
}

void ImageMovingAverage::execute() {
    auto input = getInputData<Image>(0);
    auto output = getOutputData<Image>(0);

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto program = getOpenCLProgram(device);

    if(m_keepDataType) {
        output->create(input->getSize(), input->getDataType(), 1);
    } else {
        output->create(input->getSize(), TYPE_FLOAT, 1);
    }
    SceneGraph::setParentNode(output, input);
    output->setSpacing(input->getSpacing());

    if(m_buffer.empty()) {
        // Fill buffer with duplicates
        if(!m_memory) {
            m_memory = Image::New();
            m_memory->create(input->getSize(), TYPE_FLOAT, 1);
        }

        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        auto memoryOutAccess = m_memory->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        cl::Kernel kernel(program, "MAinitialize");
        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, *memoryOutAccess->get2DImage());
        kernel.setArg(3, (int)m_frameCount);

        device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
        );

        while(m_buffer.size() < m_frameCount)
            m_buffer.push_back(input);

        return;
    }

    m_buffer.push_back(input);

    if(input->getDimensions() != 2)
        throw Exception("ImageMovingAverage only supports 2D atm");
    if (input->getNrOfChannels() > 1)
        reportWarning() << "ImageMovingAverage only supports single channel images" << reportEnd();

    if(input->getSize() != m_memory->getSize())
        throw Exception("Image input to ImageMovingAverage suddenly changed size.");

    auto memoryOut = Image::New();
    memoryOut->createFromImage(m_memory);

    auto last = m_buffer.front();

    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto lastAccess = last->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    auto memoryAccess = m_memory->getOpenCLImageAccess(ACCESS_READ, device);
    auto memoryOutAccess = memoryOut->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    cl::Kernel kernel(program, "MAiteration");
    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *memoryAccess->get2DImage());
    kernel.setArg(2, *lastAccess->get2DImage());
    kernel.setArg(3, *outputAccess->get2DImage());
    kernel.setArg(4, *memoryOutAccess->get2DImage());
    kernel.setArg(5, (int)m_frameCount);

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );

    if(m_buffer.size() > m_frameCount)
        m_buffer.pop_front();

    m_memory = memoryOut;
}

}
