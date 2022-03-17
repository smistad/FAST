#include "ScanConverter.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

ScanConverter::ScanConverter(int width, int height, float gain, float dynamicRange) {
    createInputPort(0, "Beamspace image");
    createOutputPort(0, "Scan converted image");
    m_width = width;
    m_height = height;
    m_gain = gain;
    m_dynamicRange = dynamicRange;
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/Ultrasound/ScanConverter.cl");
}

static void pol2cart(float r, float th, float &x, float &y) {
    x = r * std::cos(th);
    y = r * std::sin(th);
}

void ScanConverter::execute() {
    auto input = getInputData<Image>();
    auto output = Image::create(m_width, m_height, TYPE_UINT8, 1);

    float startRadius = std::stof(input->getFrameData("startRadius"));
    float stopRadius = std::stof(input->getFrameData("stopRadius"));
    float startTheta = std::stof(input->getFrameData("startTheta"));
    float stopTheta = std::stof(input->getFrameData("stopTheta"));
    float depthSpacing = std::stof(input->getFrameData("depthSpacing"));
    float azimuthSpacing = std::stof(input->getFrameData("azimuthSpacing"));

    float startX, startY, stopX, stopY, notUsed;
    if(input->getFrameData("isPolar") == "true") {
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
        kernel.setArg(10, depthSpacing);
        kernel.setArg(11, azimuthSpacing);
        kernel.setArg(12, (int)(input->getFrameData("isPolar") == "true" ? 1 : 0));

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
        kernel.setArg(8, depthSpacing);
        kernel.setArg(9, azimuthSpacing);
        kernel.setArg(10, (int)(input->getFrameData("isPolar") == "true" ? 1 : 0));

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