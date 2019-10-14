#include "NonLocalMeansNew.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

    NonLocalMeansNew::NonLocalMeansNew() {
        createInputPort<Image>(0);
        createOutputPort<Image>(0);

        createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NonLocalMeans/NonLocalMeansNew2D.cl");
    }

    void NonLocalMeansNew::execute() {
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

        bool preProcess = true;

        auto accessInput = input->getOpenCLImageAccess(ACCESS_READ, device);
        auto accessOutput = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        auto accessAux = auxImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);

        auto bufferIn = accessInput->get2DImage();
        auto bufferOut = accessAux->get2DImage();

        if (preProcess) {
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

        float parameterH = 0.1f;
        for (int iteration = 0; iteration < 3; ++iteration) {
            kernelNLM.setArg(0, *bufferIn);
            kernelNLM.setArg(1, *bufferOut);
            kernelNLM.setArg(2, 9);
            kernelNLM.setArg(3, 3);
            kernelNLM.setArg(4, parameterH*(1.0f/(float)std::pow(2, iteration)));
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
    }
}