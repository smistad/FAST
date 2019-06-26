#include <FAST/Data/Image.hpp>
#include "PatchStitcher.hpp"

namespace fast {

PatchStitcher::PatchStitcher() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/ImagePatch/PatchStitcher.cl");
}

void PatchStitcher::execute() {
    auto patch = getInputData<Image>();
    const int fullWidth = std::stoi(patch->getMetadata("original-width"));
    const int fullHeight = std::stoi(patch->getMetadata("original-height"));

    if(!m_outputImage) {
        // Create output image
        m_outputImage = Image::New();
        m_outputImage->create(fullWidth, fullHeight, patch->getDataType(), patch->getNrOfChannels());
        m_outputImage->fill(0);
    }

    const int startX = std::stoi(patch->getMetadata("patchid-x"))*std::stoi(patch->getMetadata("patch-width"));
    const int startY = std::stoi(patch->getMetadata("patchid-y"))*std::stoi(patch->getMetadata("patch-height"));
    const int endX = startX + patch->getWidth();
    const int endY = startY + patch->getHeight();

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Program program = getOpenCLProgram(device);

    auto patchAccess = patch->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = m_outputImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    cl::Kernel kernel(program, "applyPatch");
    kernel.setArg(0, *patchAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, startX);
    kernel.setArg(3, startY);
    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(patch->getWidth(), patch->getHeight()),
            cl::NullRange
    );

    addOutputData(0, m_outputImage);
}


}