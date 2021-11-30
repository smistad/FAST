#include "EnvelopeAndLogCompressor.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

EnvelopeAndLogCompressor::EnvelopeAndLogCompressor() {
    createInputPort(0, "");
    createOutputPort(0, "");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Ultrasound/EnvelopeAndLogCompressor.cl");
}

void EnvelopeAndLogCompressor::execute() {
    auto input = getInputData<Image>();
    auto output = Image::create(input->getWidth(), input->getHeight(), TYPE_FLOAT, 1);

    std::complex<float> maxValue;
    {
        // TODO This is slow. Get max using GPU instead
        auto access = input->getImageAccess(ACCESS_READ);
        maxValue = std::complex<float>(access->getScalarFast<float>(0, 0), access->getScalarFast<float>(0, 1));
        for(int i = 0; i < input->getNrOfVoxels(); ++i) {
            std::complex<float> value(access->getScalarFast<float>(i, 0), access->getScalarFast<float>(i, 1));
            if(std::abs(value) > std::abs(maxValue))
                maxValue = value;
        }
    }

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::Kernel kernel(getOpenCLProgram(device), "envelopeAndLogCompress");

    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    kernel.setArg(0, *inputAccess->get2DImage());
    kernel.setArg(1, *outputAccess->get2DImage());
    kernel.setArg(2, maxValue.real());
    kernel.setArg(3, maxValue.imag());

    device->getCommandQueue().enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
            );

    addOutputData(0, output);
}

}