#include "BinaryThresholding.hpp"
#include "DeviceManager.hpp"

namespace fast {

void BinaryThresholding::setInput(ImageData::pointer input) {
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

void BinaryThresholding::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
    mIsModified = true;
}

ImageData::pointer BinaryThresholding::getOutput() {
    if(!mOutput.isValid()) {
        throw Exception("Must call setInput before getOutput in Skeletonization algorithm.");
    }
    return mOutput;
}

void BinaryThresholding::setLowerThreshold(float threshold) {
    mLowerThreshold = threshold;
    mLowerThresholdSet = true;
    mIsModified = true;
}

void BinaryThresholding::setUpperThreshold(float threshold) {
    mUpperThreshold = threshold;
    mUpperThresholdSet = true;
    mIsModified = true;
}

BinaryThresholding::BinaryThresholding() {
    mLowerThresholdSet = false;
    mUpperThresholdSet = false;
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
}

void BinaryThresholding::execute() {
    if(!mLowerThresholdSet && !mUpperThresholdSet) {
        throw Exception("BinaryThresholding need at least one threshold to be set.");
    }
    if(!mInput.isValid()) {
        throw Exception("No input supplied to BinaryThresholding algorithm.");
    }
    if(!mOutput.isValid()) {
        // output object is no longer valid
        return;
    }
    Image::pointer input;
    if(mInput->isDynamicData()) {
        input = DynamicImage::pointer(mInput)->getNextFrame(mPtr);
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
    if(input->getDimensions() == 2) {
        output->create2DImage(
                input->getWidth(),
                input->getHeight(),
                TYPE_UINT8,
                1,
                mDevice
                );
    } else {
        throw Exception("Not implemented yet.");
         output->create3DImage(
                input->getWidth(),
                input->getHeight(),
                input->getDepth(),
                TYPE_UINT8,
                1,
                mDevice
                );
    }

    if(mDevice->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(mDevice);
        int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/BinaryThresholding/BinaryThresholding2D.cl");
        cl::Program program = device->getProgram(programNr);
        cl::Kernel kernel;
        if(mLowerThresholdSet && mUpperThresholdSet) {
            kernel = cl::Kernel(program, "tresholding");
            kernel.setArg(2, mLowerThreshold);
            kernel.setArg(3, mUpperThreshold);
        } else if(mLowerThresholdSet) {
            kernel = cl::Kernel(program, "thresholdingWithOnlyLower");
            kernel.setArg(2, mLowerThreshold);
        } else {
            kernel = cl::Kernel(program, "thresholdingWithOnlyUpper");
            kernel.setArg(2, mUpperThreshold);
        }
        OpenCLImageAccess2D access = input->getOpenCLImageAccess2D(ACCESS_READ, device);
        OpenCLImageAccess2D access2 = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *access.get());
        kernel.setArg(1, *access2.get());

        cl::CommandQueue queue = device->getCommandQueue();
        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(output->getWidth(), output->getHeight()),
                cl::NullRange
        );
    }

    output->updateModifiedTimestamp();
}

void BinaryThresholding::waitToFinish() {
    OpenCLDevice::pointer device = OpenCLDevice::pointer(mDevice);
    device->getCommandQueue().finish();
}

} // end namespace fast
