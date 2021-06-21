#include "BlockMatching.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

void BlockMatching::loadAttributes() {
    setBlockSize(getIntegerAttribute("block-size"));
    setSearchSize(getIntegerAttribute("search-size"));
    setIntensityThreshold(getFloatAttribute("intensity-threshold"));
    setMatchingMetric(BlockMatching::stringToMetric(getStringAttribute("metric")));
    setTimeLag(getIntegerAttribute("time-lag"));
    setForwardBackwardTracking(getBooleanAttribute("forward-backward"));
    auto roiOffset = getIntegerListAttribute("roi-offset");
    auto roiSize = getIntegerListAttribute("roi-size");
    if(roiOffset.size() == 2 && roiSize.size() == 2) {
        setRegionOfInterest(Vector2i(roiOffset[0], roiOffset[1]),
            Vector2i(roiSize[0], roiSize[1]));
    }
}

BlockMatching::BlockMatching(int blockSize, int searhSize, MatchingMetric metric, bool forwardBackwardTracking, int timeLag) {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/BlockMatching/BlockMatching.cl");

    createIntegerAttribute("block-size", "Block size", "Block size", m_blockSizeHalf * 2 + 1);
    createIntegerAttribute("search-size", "Search size", "Search size", m_searchSizeHalf * 2 + 1);
    createIntegerAttribute("time-lag", "Time lag", "Time lag", m_timeLag);
    createFloatAttribute("intensity-threshold", "Intensity threshold", "Pixels with an intensity below this threshold will not be processed", m_intensityThreshold);
    createStringAttribute("metric", "Matching metric", "Possible values are SSD, SAD, and NCC", "SAD");
    createBooleanAttribute("forward-backward", "Forward-backward tracking", "Do tracking forward and backwards and take the average.", m_forwardBackward);
    createIntegerAttribute("roi-offset", "ROI offset", "Offset of region of interest (ROI)", 0);
    createIntegerAttribute("roi-size", "ROI size", "Size of region of interest (ROI), 0 0 means no ROI is used.", 0);

    setBlockSize(blockSize);
    setSearchSize(searhSize);
    setMatchingMetric(metric);
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

    auto output = Image::create(currentFrame->getSize(), TYPE_FLOAT, 2);
    output->setSpacing(currentFrame->getSpacing());
    m_frameBuffer.push_back(currentFrame);

    if(m_frameBuffer.size() < m_timeLag+1) {
        // If previous frame is not available, just fill it with zeros and stop
        output->fill(0);
        addOutputData(0, output);
        return;
    }


    std::string buildOptions = "-DGRID_SIZE=" + std::to_string(m_searchSizeHalf*2 + 3) + " "
                               "-DBLOCK_SIZE=" + std::to_string(m_blockSizeHalf) + " "
                               "-DSEARCH_SIZE=" + std::to_string(m_searchSizeHalf);
    cl::Kernel kernel(getOpenCLProgram(device, "", buildOptions), kernelNames.at(m_type).c_str());
    auto queue = device->getCommandQueue();

    if(m_type == MatchingMetric::SUM_OF_SQUARED_DIFFERENCES || m_type == MatchingMetric::SUM_OF_ABSOLUTE_DIFFERENCES) {
        float max = currentFrame->calculateMaximumIntensity();
        float min = currentFrame->calculateMinimumIntensity();
        kernel.setArg(6, min);
        kernel.setArg(7, max);
    }

    auto previousFrame = m_frameBuffer.front();
    auto previousFrameAccess = previousFrame->getOpenCLImageAccess(ACCESS_READ, device);
    auto currentFrameAccess = currentFrame->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    kernel.setArg(0, *previousFrameAccess->get2DImage());
    kernel.setArg(1, *currentFrameAccess->get2DImage());
    kernel.setArg(2, *outputAccess->get2DImage());
    kernel.setArg(3, m_intensityThreshold);
    kernel.setArg(4, (float)m_timeLag);
    kernel.setArg(5, (char)(m_forwardBackward ? 1 : 0));

    if(m_sizeROI == Vector2i::Zero()) {
        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(output->getWidth(), output->getHeight()),
            cl::NullRange
        );
    } else {
        queue.enqueueNDRangeKernel(
            kernel,
            cl::NDRange(m_offsetROI.x(), m_offsetROI.y()),
            cl::NDRange(m_sizeROI.x(), m_sizeROI.y()),
            cl::NullRange
        );
    }
    queue.finish();
    m_frameBuffer.pop_front();
    addOutputData(0, output);
}

void BlockMatching::setMatchingMetric(MatchingMetric type) {
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

void BlockMatching::setTimeLag(int timeLag) {
    if(timeLag < 1)
        throw Exception("Time lag must be >= 1");

    m_timeLag = timeLag;
}

void BlockMatching::setForwardBackwardTracking(bool forwardBackward) {
    m_forwardBackward = forwardBackward;
}

void BlockMatching::setRegionOfInterest(Vector2i offset, Vector2i size) {
    if(offset.x() < 0 || offset.y() < 0)
        throw Exception("Offset ROI must >= 0");
    if(size.x() < 0 || size.y() < 0)
        throw Exception("Size ROI must be > 0");
    m_offsetROI = offset;
    m_sizeROI = size;
}

}