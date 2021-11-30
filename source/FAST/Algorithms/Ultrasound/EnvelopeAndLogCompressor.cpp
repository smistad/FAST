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

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);

    {
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        cl::Kernel kernel(getOpenCLProgram(device), "envelopeAndLogCompress");
        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
                );
    }

    if(m_maxInitialize) {
        m_maxValue = std::max(m_maxValue, output->calculateMaximumIntensity());
    } else {
        m_maxValue = output->calculateMaximumIntensity();
        m_maxInitialize = true;
    }

    {
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ, device);
        auto normalizedOutput = Image::createFromImage(output);
        auto normalizedOutputAccess = normalizedOutput->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        cl::Kernel kernel(getOpenCLProgram(device), "normalizeEnvelope");
        kernel.setArg(0, *outputAccess->get2DImage());
        kernel.setArg(1, *normalizedOutputAccess->get2DImage());
        kernel.setArg(2, m_maxValue);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
        );
        addOutputData(0, normalizedOutput);
    }

}

}