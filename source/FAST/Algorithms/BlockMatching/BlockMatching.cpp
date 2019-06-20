#include "BlockMatching.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

BlockMatching::BlockMatching() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/BlockMatching/BlockMatching.cl");
}

void BlockMatching::execute() {
    auto currentFrame = getInputData<Image>(0);

    std::map<MatchingMetric, std::string> kernelNames = {
            {MatchingMetric::NORMALIZED_CROSS_CORRELATION, "normalizedCrossCorrelation"},
            {MatchingMetric::SUM_OF_SQUARED_DIFFERENCES, "sumOfSquaredDifferences"},
            {MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES, "sumOfAbsoluteDifferences"},
    };

    if(currentFrame->getDimensions() != 2)
        throw Exception("Block matching only implemented for 2D");

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    auto output = getOutputData<Image>(0);
    output->create(currentFrame->getSize(), TYPE_FLOAT, 2);
    output->setSpacing(currentFrame->getSpacing());
    if(!m_previousFrame) {
        // If previous frame is not available, just fill it with zeros and stop
        output->fill(0);
        m_previousFrame = currentFrame;
        return;
    }


    std::string buildOptions = "-DSEARCH_SIZE=" + std::to_string(m_searchSizeHalf*2 + 3);
    cl::Kernel kernel(getOpenCLProgram(device, "", buildOptions), kernelNames.at(m_type).c_str());
    auto queue = device->getCommandQueue();

    if(m_type == MatchingMetric::SUM_OF_SQUARED_DIFFERENCES || m_type == MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES) {
        float max = currentFrame->calculateMaximumIntensity();
        float min = currentFrame->calculateMinimumIntensity();
        kernel.setArg(6, min);
        kernel.setArg(7, max);
    }

    auto previousFrameAccess = m_previousFrame->getOpenCLImageAccess(ACCESS_READ, device);
    auto currentFrameAccess = currentFrame->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    kernel.setArg(0, *previousFrameAccess->get2DImage());
    kernel.setArg(1, *currentFrameAccess->get2DImage());
    kernel.setArg(2, *outputAccess->get2DImage());
    kernel.setArg(3, m_blockSizeHalf);
    kernel.setArg(4, m_searchSizeHalf);
    kernel.setArg(5, m_intensityThreshold);

    queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,
        cl::NDRange(output->getWidth(), output->getHeight()),
        cl::NullRange
    );
    queue.finish();

    m_previousFrame = currentFrame;
}

void BlockMatching::setMatchingMetric(BlockMatching::MatchingMetric type) {
    m_type = type;
}

void BlockMatching::setBlockSize(int size) {
    if(size < 3 || size % 2 == 0)
        throw Exception("Block size has to be >= 3 and odd");

    m_blockSizeHalf = (size-1)/2;
}

void BlockMatching::setSearchSize(int size) {
    if(size < 3 || size % 2 == 0)
        throw Exception("Search size has to be >= 3 and odd");

    m_searchSizeHalf = (size-1)/2;
}

void BlockMatching::setIntensityThreshold(float value) {
    m_intensityThreshold = value;
}

}