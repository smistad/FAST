#include "ScanConverter.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ScanConverter::ScanConverter(int width, int height, float gain, float dynamicRange, float startDepth, float endDepth, float startAngle, float endAngle, float leftPos, float rightPos) {
    createInputPort(0, "Beamspace image");
    createOutputPort(0, "Scan converted image");

    m_width = width;
    m_height = height;
    m_gain = gain;
    m_dynamicRange = dynamicRange;
    m_startDepth = startDepth;
    m_endDepth = endDepth;
    m_startAngle = startAngle;
    m_endAngle = endAngle;
    m_leftPos = leftPos;
    m_rightPos = rightPos;

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Ultrasound/ScanConverter.cl");
}

static void pol2cart(float r, float th, float &x, float &y) {
    x = r * std::cos(th);
    y = r * std::sin(th);
}

void ScanConverter::execute() {
    auto input = getInputData<Image>();
    auto output = Image::create(m_width, m_height, TYPE_UINT8, 1);

    float startRadius = m_startDepth;
    float stopRadius = m_endDepth;
    if(m_endDepth - m_startDepth <= 0) {
        startRadius = std::stof(input->getFrameData("startRadius"));
        stopRadius = std::stof(input->getFrameData("stopRadius"));
    }
    float startTheta;
    float stopTheta;
    bool isPolar = false;
    if(m_endAngle - m_startAngle > 0) {
        isPolar = true;
        startTheta = m_startAngle;
        stopTheta = m_endAngle;
    } else if(m_rightPos - m_leftPos > 0) {
        isPolar = false;
        startTheta = m_leftPos;
        stopTheta = m_rightPos;
    } else {
        startTheta = std::stof(input->getFrameData("startTheta"));
        stopTheta = std::stof(input->getFrameData("stopTheta"));
        isPolar = input->getFrameData("isPolar") == "true";
    }

    float startX, startY, stopX, stopY, notUsed;
    if(isPolar) {
        pol2cart(startRadius, startTheta, startY, notUsed);
        pol2cart(stopRadius, startTheta, notUsed, startX);
        pol2cart(stopRadius, 0, stopY, notUsed);
        pol2cart(stopRadius, stopTheta, notUsed, stopX);
    } else {
        startY = startRadius;
        stopY = stopRadius;
        startX = startTheta;
        stopX = stopTheta;
    }

    float newXSpacing = (stopX - startX) / (m_width - 1); //Subtract 1 because num spaces is 1 less than num elements
    float newYSpacing = (stopY - startY) / (m_height - 1);
    output->setSpacing(newXSpacing, newYSpacing, 1.0f);

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    if(input->getDataType() == TYPE_FLOAT) {
        cl::Kernel kernel(getOpenCLProgram(device), "scanConvert");

        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, m_gain);
        kernel.setArg(3, m_dynamicRange);
        kernel.setArg(4, newXSpacing);
        kernel.setArg(5, newYSpacing);
        kernel.setArg(6, startX);
        kernel.setArg(7, startY);
        kernel.setArg(8, startRadius);
        kernel.setArg(9, startTheta);
        kernel.setArg(10, (stopRadius-startRadius)/input->getHeight());
        kernel.setArg(11, (stopTheta-startTheta)/input->getWidth());
        kernel.setArg(12, (int)(isPolar ? 1 : 0));

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(m_width, m_height),
                cl::NullRange
        );
    } else {
        // Already grayscale..
        cl::Kernel kernel(getOpenCLProgram(device), "scanConvertGrayscale");

        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, newXSpacing);
        kernel.setArg(3, newYSpacing);
        kernel.setArg(4, startX);
        kernel.setArg(5, startY);
        kernel.setArg(6, startRadius);
        kernel.setArg(7, startTheta);
        kernel.setArg(8, (stopRadius-startRadius)/input->getHeight());
        kernel.setArg(9, (stopTheta-startTheta)/input->getWidth());
        kernel.setArg(10, (int)(isPolar ? 1 : 0));

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(m_width, m_height),
                cl::NullRange
        );
    }

    addOutputData(0, output);

}

void ScanConverter::setGain(float gain) {
    m_gain = gain;
    setModified(true);
}

void ScanConverter::setDynamicRange(float dynamicRange) {
    m_dynamicRange = dynamicRange;
    setModified(true);
}

void ScanConverter::setWidth(int width) {
    m_width = width;
    setModified(true);
}

void ScanConverter::setHeight(int height) {
    m_height = height;
    setModified(true);
}

int ScanConverter::getWidth() const {
    return m_width;
}

int ScanConverter::getHeight() const {
    return m_height;
}

}