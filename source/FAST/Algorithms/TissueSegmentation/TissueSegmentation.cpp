#include <FAST/Data/ImagePyramid.hpp>
#include "TissueSegmentation.hpp"
#include <FAST/Algorithms/Morphology/Dilation.hpp>
#include <FAST/Algorithms/Morphology/Erosion.hpp>
#include <FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp>

namespace fast {

TissueSegmentation::TissueSegmentation(int threshold, int dilationSize, int erosionSize, bool filterZeros) {
    createInputPort<ImagePyramid>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Algorithms/TissueSegmentation/TissueSegmentation.cl");
    createIntegerAttribute("threshold", "Intensity threshold", "", m_thresh);
    createIntegerAttribute("dilate-kernel-size", "Kernel size for dilation", "", m_dilate);
    createIntegerAttribute("erode-kernel-size", "Kernel size for erosion", "", m_erode);
    createBooleanAttribute("filter-zeros", "Include zero values to background/glass class", "", m_filterZeros);
    setThreshold(threshold);
    setDilate(dilationSize);
    setErode(erosionSize);
    setFilterZeros(filterZeros);
}

void TissueSegmentation::loadAttributes() {
    setThreshold(getIntegerAttribute("threshold")); 
    setDilate(getIntegerAttribute("dilate-kernel-size"));
    setErode(getIntegerAttribute("erode-kernel-size"));
    setFilterZeros(getBooleanAttribute("filter-zeros"));

}

void TissueSegmentation::setThreshold(int thresh) {
    m_thresh = thresh;
}

void TissueSegmentation::setDilate(int radius) {
    m_dilate = radius;
}

void TissueSegmentation::setErode(int radius) {
    m_erode = radius;
}

void TissueSegmentation::setFilterZeros(bool value) {
    m_filterZeros = value;
}

int TissueSegmentation::getThreshold() const {
    return m_thresh;
}

int TissueSegmentation::getErode() const {
    return m_erode;
}

int TissueSegmentation::getDilate() const {
    return m_dilate;
}

bool TissueSegmentation::getFilterZeros() const {
    return m_filterZeros;
}

void TissueSegmentation::execute() {
    auto wsi = getInputData<ImagePyramid>();
    auto access = wsi->getAccess(ACCESS_READ);
    auto input = access->getLevelAsImage(wsi->getNrOfLevels()-1);

    auto output = Image::createSegmentationFromImage(input);

    {
        auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        auto program = getOpenCLProgram(device);
        cl::Kernel kernel(program, "segment");

        auto inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *inputAccess->get2DImage());
        kernel.setArg(1, *outputAccess->get2DImage());
        kernel.setArg(2, m_thresh);
        kernel.setArg(3, (int) m_filterZeros);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
        );
        device->getCommandQueue().finish();
    }

    if ((m_dilate == 0) && (m_erode == 0)) {
        addOutputData(0, output);  // no morphological post-processing

    } else if ((m_dilate > 0) && (m_erode == 0)){
        auto dilation = Dilation::New();
        dilation->setInputData(output);
        dilation->setStructuringElementSize(m_dilate);

        auto newOutput = dilation->updateAndGetOutputData<Image>();
        addOutputData(0, newOutput);

    } else if ((m_dilate == 0) && (m_erode > 0)){
        auto erosion = Erosion::New();
        erosion->setInputData(output);
        erosion->setStructuringElementSize(m_erode);

        auto newOutput = erosion->updateAndGetOutputData<Image>();
        addOutputData(0, newOutput);

    } else {
        auto dilation = Dilation::New();
        dilation->setInputData(output);
        dilation->setStructuringElementSize(m_dilate);

        auto erosion = Erosion::New();
        erosion->setInputConnection(dilation->getOutputPort());
        erosion->setStructuringElementSize(m_erode);

        auto newOutput = erosion->updateAndGetOutputData<Image>();
        addOutputData(0, newOutput);  // closing (instead of opening) to increase sensitivity in detection
    }

}

}