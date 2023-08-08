#include "EnvelopeAndLogCompressor.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

EnvelopeAndLogCompressor::EnvelopeAndLogCompressor(bool convertToGrayscale, float gain, float dynamicRange) {
    createInputPort(0, "");
    createOutputPort(0, "");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Ultrasound/EnvelopeAndLogCompressor.cl");
    setConvertToGrayscale(convertToGrayscale);
    setGain(gain);
    setDynamicRange(dynamicRange);
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

        cl::Kernel kernel;
        Image::pointer normalizedOutput;
        if(m_convertToGrayscale) {
            normalizedOutput = Image::create(output->getSize(), TYPE_UINT8, 1);
            auto normalizedOutputAccess = normalizedOutput->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            kernel = cl::Kernel(getOpenCLProgram(device), "normalizeEnvelopeToGrayscale");
            kernel.setArg(0, *outputAccess->get2DImage());
            kernel.setArg(1, *normalizedOutputAccess->get2DImage());
            kernel.setArg(2, m_maxValue);
            kernel.setArg(3, m_gain);
            kernel.setArg(4, m_dynamicRange);
        } else {
            normalizedOutput = Image::createFromImage(output);
            auto normalizedOutputAccess = normalizedOutput->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            kernel = cl::Kernel(getOpenCLProgram(device), "normalizeEnvelope");
            kernel.setArg(0, *outputAccess->get2DImage());
            kernel.setArg(1, *normalizedOutputAccess->get2DImage());
            kernel.setArg(2, m_maxValue);
        }

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
        );
        addOutputData(0, normalizedOutput);
    }

}

void EnvelopeAndLogCompressor::setConvertToGrayscale(bool convert) {
    m_convertToGrayscale = convert;
    setModified(true);
}

void EnvelopeAndLogCompressor::setGain(float gain) {
    m_gain = gain;
    setModified(true);
}

void EnvelopeAndLogCompressor::setDynamicRange(float dynamicRange) {
    m_dynamicRange = dynamicRange;
    setModified(true);
}

}