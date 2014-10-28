#include "Skeletonization.hpp"
#include "DeviceManager.hpp"
#include "HelperFunctions.hpp"

namespace fast {

void Skeletonization::setInput(ImageData::pointer input) {
    mInput = input;
    setParent(input);
    mIsModified = true;
    if(input->isDynamicData()) {
        mOutput = DynamicImage::New();
        DynamicImage::pointer(mOutput)->setStreamer(DynamicImage::pointer(mInput)->getStreamer());
    } else {
        mOutput = Image::New();
        input->retain(mDevice);
    }
    mOutput->setSource(mPtr.lock());
}

ImageData::pointer Skeletonization::getOutput() {
    if(!mOutput.isValid()) {
        throw Exception("Must call setInput before getOutput in Skeletonization algorithm.");
    }
    return mOutput;
}

Skeletonization::Skeletonization() {
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
}

void Skeletonization::execute() {
    Image::pointer input;
    if(!mInput.isValid()) {
        throw Exception("No input supplied to Skeletonization algorithm.");
    }
    if(!mOutput.isValid()) {
        // output object is no longer valid
        return;
    }
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame();
    } else {
        input = mInput;
    }

    Image::pointer output;
    if(mInput->isDynamicData()) {
        output = Image::New();
        DynamicImage::pointer(mOutput)->addFrame(output);
    } else {
        output = Image::pointer(mOutput);
    }

    // Initialize output image
    output->create2DImage(
            input->getWidth(),
            input->getHeight(),
            TYPE_UINT8,
            1,
            mDevice
            );

    // Create kernel
    int programNr = mDevice->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/Skeletonization/Skeletonization2D.cl");
    cl::Program program = mDevice->getProgram(programNr);
    cl::Kernel kernelInit(program, "initialize");
    cl::Kernel kernel1(program, "thinningStep1");
    cl::Kernel kernel2(program, "thinningStep2");

    // Create buffer for check if stop
    cl::Buffer stopGrowingBuffer = cl::Buffer(
            mDevice->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(char));

    cl::Image2D image2(mDevice->getContext(), CL_MEM_READ_WRITE, cl::ImageFormat(CL_R, CL_UNSIGNED_INT8), output->getWidth(), output->getHeight());
    OpenCLImageAccess2D access = input->getOpenCLImageAccess2D(ACCESS_READ, mDevice);
    cl::Image2D* image = access.get();
    OpenCLImageAccess2D access2 = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, mDevice);
    cl::Image2D* image1 = access2.get();

    float threshold = 0.5;
    kernelInit.setArg(0, *image);
    kernelInit.setArg(1, *image1);
    kernelInit.setArg(2, image2);
    kernelInit.setArg(3, threshold);

    kernel1.setArg(0, *image1);
    kernel1.setArg(1, image2);
    kernel1.setArg(2, stopGrowingBuffer);

    kernel2.setArg(0, image2);
    kernel2.setArg(1, *image1);
    kernel2.setArg(2, stopGrowingBuffer);

    cl::NDRange globalSize(output->getWidth(), output->getHeight());

    bool stopGrowing = false;
    char stopGrowingInit = 1;
    char* stopGrowingResult = new char;
    int iterations = 0;
    cl::CommandQueue queue = mDevice->getCommandQueue();
    queue.enqueueNDRangeKernel(
            kernelInit,
            cl::NullRange,
            globalSize,
            cl::NullRange
    );

    do {
        iterations++;
        queue.enqueueWriteBuffer(stopGrowingBuffer, CL_FALSE, 0, sizeof(char), &stopGrowingInit);

        queue.enqueueNDRangeKernel(
                kernel1,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );

        queue.enqueueNDRangeKernel(
                kernel2,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );

        queue.enqueueReadBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), stopGrowingResult);
        if(*stopGrowingResult == 1)
            stopGrowing = true;
    } while(!stopGrowing);

    output->updateModifiedTimestamp();
}

} // end namespace fast
