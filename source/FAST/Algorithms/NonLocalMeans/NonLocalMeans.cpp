#include "NonLocalMeans.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

NonLocalMeans::NonLocalMeans() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NonLocalMeans/NonLocalMeans2D.cl");
}

void NonLocalMeans::execute() {
    auto input = getInputData<Image>(0);
    auto output = getOutputData<Image>(0);
    output->createFromImage(input);
    auto auxImage = Image::New();
    auxImage->createFromImage(input);

    const int width = input->getWidth();
    const int height = input->getHeight();

    auto device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto program = getOpenCLProgram(device);
    auto queue = device->getCommandQueue();

    cl::Kernel kernelPreProcess(program, "preprocess");
    cl::Kernel kernelNLM(program, "nonLocalMeansFilter");

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
    queue.finish();
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

}