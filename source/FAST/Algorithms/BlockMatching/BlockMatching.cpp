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

    if(currentFrame->getDimensions() != 2)
        throw Exception("Block matching only implemented for 2D");

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    auto output = getOutputData<Image>(0);
    output->create(currentFrame->getSize(), TYPE_FLOAT, 3);
    output->setSpacing(currentFrame->getSpacing());
    if(!m_previousFrame) {
        // If previous frame is not available, just fill it with zeros and stop
        output->fill(0);
        m_previousFrame = currentFrame;
        return;
    }


    cl::Kernel kernel(getOpenCLProgram(device), "blockMatching");
    auto queue = device->getCommandQueue();

    auto previousFrameAccess = m_previousFrame->getOpenCLImageAccess(ACCESS_READ, device);
    auto currentFrameAccess = currentFrame->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    int blockSizeHalf = 5;
    int searchSizeHalf = 2;

    kernel.setArg(0, *previousFrameAccess->get2DImage());
    kernel.setArg(1, *currentFrameAccess->get2DImage());
    kernel.setArg(2, *outputAccess->get2DImage());
    kernel.setArg(3, blockSizeHalf);
    kernel.setArg(4, searchSizeHalf);

    queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,
        cl::NDRange(output->getWidth(), output->getHeight()),
        cl::NullRange
    );

    m_previousFrame = currentFrame;
}

}