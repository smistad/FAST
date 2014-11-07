#include "ImageGradient.hpp"
#include "DeviceManager.hpp"

namespace fast {

void ImageGradient::setInput(ImageData::pointer input) {
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

ImageData::pointer ImageGradient::getOutput() {
    if(!mOutput.isValid()) {
        throw Exception("Must call setInput before getOutput in ImageGradient algorithm.");
    }
    return mOutput;
}

ImageGradient::ImageGradient() {
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
}

void ImageGradient::execute() {
    if(!mInput.isValid()) {
        throw Exception("No input supplied to BinaryThresholding algorithm.");
    }
    if(!mOutput.isValid()) {
        // output object is no longer valid
        return;
    }
    Image::pointer input;
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
    if(input->getDimensions() == 2) {
        output->create2DImage(
                input->getWidth(),
                input->getHeight(),
                TYPE_FLOAT,
                2,
                mDevice
                );
    } else {
        throw Exception("Not implemented yet.");
         output->create3DImage(
                input->getWidth(),
                input->getHeight(),
                input->getDepth(),
                TYPE_FLOAT,
                3,
                mDevice
                );
    }

    if(mDevice->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(mDevice);
        std::string buildOptions = "";
        if(input->getDataType() == TYPE_FLOAT) {
            buildOptions = "-DTYPE_FLOAT";
        } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
            buildOptions = "-DTYPE_INT";
        } else {
            buildOptions = "-DTYPE_UINT";
        }
        int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/ImageGradient/ImageGradient.cl", buildOptions);
        cl::Program program = device->getProgram(programNr);
        cl::Kernel kernel;
        if(input->getDimensions() == 2) {
            kernel = cl::Kernel(program, "gradient2D");
            OpenCLImageAccess2D inputAccess = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            OpenCLImageAccess2D outputAccess = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *inputAccess.get());
            kernel.setArg(1, *outputAccess.get());
        } else {
            kernel = cl::Kernel(program, "gradient3D");
            OpenCLImageAccess3D inputAccess = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            OpenCLImageAccess3D outputAccess = output->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *inputAccess.get());
            kernel.setArg(1, *outputAccess.get());
        }

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth()),
                cl::NullRange
        );
    }

    output->updateModifiedTimestamp();
}


}

