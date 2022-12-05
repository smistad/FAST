#include "NonLocalMeans.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

void NonLocalMeans::init() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NonLocalMeans/NonLocalMeans2D.cl");

    createFloatAttribute("smoothing", "Smoothing amount", "Controls how much smoothing to apply", 0.15);
    createFloatAttribute("input-multiplication-weight", "Input image multiplication weight", "The input image is multipled with the NLM filteret image to recover some fine-grained details."
                                                                                             "This weight determines how strongly this should be multipled in. With a value of zero, the input image is not multiplied in at all.", m_multiplicationWeight);
    createIntegerAttribute("search-size", "Search size", "How big pixel area to search", 11);
    createIntegerAttribute("filter-size", "Filter size", "Filter size", 3);
    createIntegerAttribute("iterations", "Iterations", "Number of multiscale iterations", 3);
    createBooleanAttribute("preprocess", "Preprocess", "Apply preprocessing (5x5 median filter) or not", true);
}

NonLocalMeans::NonLocalMeans(int filterSize, int searchSize, float smoothingAmount, float multiplicationWeight, int multiScaleIterations) {
    init();
    setFilterSize(filterSize);
    setSearchSize(searchSize);
    setSmoothingAmount(smoothingAmount);
    setMultiscaleIterations(multiScaleIterations);
    setInputMultiplicationWeight(multiplicationWeight);
}

void NonLocalMeans::loadAttributes() {
    setSmoothingAmount(getFloatAttribute("smoothing"));
    setSearchSize(getIntegerAttribute("search-size"));
    setFilterSize(getIntegerAttribute("filter-size"));
    setMultiscaleIterations(getIntegerAttribute("iterations"));
    setPreProcess(getBooleanAttribute("preprocess"));
    setInputMultiplicationWeight(getFloatAttribute("input-multiplication-weight"));
}

void NonLocalMeans::execute() {
    auto input = getInputData<Image>(0);
    auto output = Image::createFromImage(input);
    auto auxImage = Image::createFromImage(input);

    if(input->getDataType() != TYPE_UINT8)
        throw Exception("Non local means requries UINT8 image");

    const int width = input->getWidth();
    const int height = input->getHeight();

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto program = getOpenCLProgram(device, "", 
            "-DFILTER_SIZE=" + std::to_string((m_filterSize - 1)/2) + " "
            "-DSEARCH_SIZE=" + std::to_string(m_searchSize)
            );
    auto queue = device->getCommandQueue();

    cl::Kernel kernelPreProcess(program, "preprocess");
    cl::Kernel kernelNLM(program, "nonLocalMeansFilter");
    cl::Kernel kernelMultiply(program, "multiplyWithInput");

    auto accessInput = input->getOpenCLImageAccess(ACCESS_READ, device);
    auto accessOutput = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
    auto accessAux = auxImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

    auto bufferIn = accessInput->get2DImage();
    auto bufferOut = accessAux->get2DImage();

    if(m_preProcess) {
        kernelPreProcess.setArg(0, *bufferIn);
        kernelPreProcess.setArg(1, *bufferOut);
        queue.enqueueNDRangeKernel(
            kernelPreProcess,
            cl::NullRange,
            cl::NDRange(width, height),
            cl::NullRange
        );

        bufferIn = bufferOut;
        bufferOut = accessOutput->get2DImage();
    } else {
        queue.enqueueCopyImage(
            *bufferIn,
            *bufferOut,
            createOrigoRegion(),
            createOrigoRegion(),
            createRegion(width, height, 1)
        );
        bufferIn = bufferOut;
        bufferOut = accessOutput->get2DImage();
    }

    for (int iteration = 0; iteration < m_iterations; ++iteration) {
        kernelNLM.setArg(0, *bufferIn);
        kernelNLM.setArg(1, *bufferOut);
        kernelNLM.setArg(2, m_searchSize);
        kernelNLM.setArg(3, (m_filterSize - 1)/2);
        kernelNLM.setArg(4, m_parameterH*(1.0f/(float)std::pow(2, iteration)));
        kernelNLM.setArg(5, iteration); // iteration

        queue.enqueueNDRangeKernel(
            kernelNLM,
            cl::NullRange,
            cl::NDRange(width, height),
            cl::NullRange
        );

        auto tmp = bufferIn;
        bufferIn = bufferOut;
        bufferOut = tmp;
    }
    if(m_multiplicationWeight > 0.0f) {
        auto output2 = Image::createFromImage(input);
        auto access2 = output2->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernelMultiply.setArg(0, *accessInput->get2DImage());
        kernelMultiply.setArg(1, *bufferOut);
        kernelMultiply.setArg(2, *access2->get2DImage());
        kernelMultiply.setArg(3, m_multiplicationWeight);
        queue.enqueueNDRangeKernel(
                kernelMultiply,
                cl::NullRange,
                cl::NDRange(width, height),
                cl::NullRange
                );
        queue.finish();
        addOutputData(0, output2);
    } else {
        queue.finish();
        addOutputData(0, output);
    }
}

void NonLocalMeans::setSmoothingAmount(float parameterH) {
    if(parameterH <= 0)
        throw Exception("Smoothing amount must be larger than 0");

    m_parameterH = parameterH;
}

void NonLocalMeans::setPreProcess(bool preProcess) {
    m_preProcess = preProcess;
}

void NonLocalMeans::setMultiscaleIterations(int iterations) {
    if(iterations < 1)
        throw Exception("Multiscale iterations must be larger than 0");
    m_iterations = iterations;
}

void NonLocalMeans::setSearchSize(int searchSize) {
    if(searchSize < 3 || searchSize % 2 == 0)
        throw Exception("Search size must be larger than 2 and be odd");

    m_searchSize = searchSize;
}

void NonLocalMeans::setFilterSize(int filterSize) {
    if(filterSize < 3 || filterSize % 2 == 0)
        throw Exception("Filter size must be larger than 2 and be odd");
    m_filterSize = filterSize;
}

void NonLocalMeans::setInputMultiplicationWeight(float weight) {
    m_multiplicationWeight = weight;
}

}