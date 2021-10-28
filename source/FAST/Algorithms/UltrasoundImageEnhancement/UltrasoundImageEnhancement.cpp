#include <FAST/Data/Image.hpp>
#include "UltrasoundImageEnhancement.hpp"

namespace fast {


UltrasoundImageEnhancement::UltrasoundImageEnhancement(int reject) {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/UltrasoundImageEnhancement/UltrasoundImageEnhancement.cl");

    createIntegerAttribute("reject", "Reject", "How many intensity values at bottom to reject.", 40);

    setReject(reject);
}

void UltrasoundImageEnhancement::execute() {
    Image::pointer input = getInputData<Image>(0);
    if(input->getDataType() != TYPE_UINT8) {
        throw Exception("UltrasoundImageEnhancement expects input to be of type UINT8");
    }

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    auto output = Image::create(input->getSize(), TYPE_UINT8, 3); // Make color image
    output->setSpacing(input->getSpacing());

    cl::CommandQueue queue = device->getCommandQueue();
    cl::Kernel kernel(getOpenCLProgram(device), "enhance");


    OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, m_reject);

    queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
    );
    addOutputData(0, output);
}

void UltrasoundImageEnhancement::loadAttributes() {
    setReject(getIntegerAttribute("reject"));
}

void UltrasoundImageEnhancement::setReject(int reject) {
    m_reject = reject;
}

}