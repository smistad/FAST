#include "BinaryThresholding.hpp"
#include "Segmentation.hpp"

namespace fast {

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
    setOutputDataDynamicDependsOnInputData(0, 0);
}

void BinaryThresholding::execute() {
    if(!mLowerThresholdSet && !mUpperThresholdSet) {
        throw Exception("BinaryThresholding need at least one threshold to be set.");
    }

    Image::pointer input = getStaticInputData<Image>(0);
    Segmentation::pointer output = getStaticOutputData<Segmentation>(0);

    output->createFromImage(input, getMainDevice());

    // Initialize output image
    if(input->getDimensions() == 3) {
        throw Exception("Not implemented yet.");
    }

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        std::string buildOptions = "";
        if(input->getDataType() == TYPE_FLOAT) {
            buildOptions = "-DTYPE_FLOAT";
        } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
            buildOptions = "-DTYPE_INT";
        } else {
            buildOptions = "-DTYPE_UINT";
        }
        int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/BinaryThresholding/BinaryThresholding2D.cl", buildOptions);
        cl::Program program = device->getProgram(programNr);
        cl::Kernel kernel;
        if(mLowerThresholdSet && mUpperThresholdSet) {
            kernel = cl::Kernel(program, "tresholding");
            kernel.setArg(3, mLowerThreshold);
            kernel.setArg(4, mUpperThreshold);
        } else if(mLowerThresholdSet) {
            kernel = cl::Kernel(program, "thresholdingWithOnlyLower");
            kernel.setArg(3, mLowerThreshold);
        } else {
            kernel = cl::Kernel(program, "thresholdingWithOnlyUpper");
            kernel.setArg(3, mUpperThreshold);
        }
        OpenCLImageAccess2D::pointer access = input->getOpenCLImageAccess2D(ACCESS_READ, device);
        OpenCLImageAccess2D::pointer access2 = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *access->get());
        kernel.setArg(1, *access2->get());
        kernel.setArg(2, (uchar)mLabel);

        cl::CommandQueue queue = device->getCommandQueue();
        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(output->getWidth(), output->getHeight()),
                cl::NullRange
        );
    }
}

void BinaryThresholding::waitToFinish() {
    OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
    device->getCommandQueue().finish();
}

} // end namespace fast
