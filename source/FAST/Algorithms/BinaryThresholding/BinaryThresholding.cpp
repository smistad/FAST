#include "BinaryThresholding.hpp"
#include "FAST/Data/Segmentation.hpp"

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
    createInputPort<Image>(0);
    createOutputPort<Segmentation>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void BinaryThresholding::execute() {
    if(!mLowerThresholdSet && !mUpperThresholdSet) {
        throw Exception("BinaryThresholding need at least one threshold to be set.");
    }

    Image::pointer input = getStaticInputData<Image>(0);
    Segmentation::pointer output = getStaticOutputData<Segmentation>(0);

    output->createFromImage(input, getMainDevice());

    if(getMainDevice()->isHost()) {
        throw Exception("Not implemented yet.");
    } else {
        OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
        int programNr;
        if(input->getDimensions() == 3) {
            programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/BinaryThresholding/BinaryThresholding3D.cl");
        } else {
            programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + "Algorithms/BinaryThresholding/BinaryThresholding2D.cl");
        }
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
        cl::NDRange globalSize;
        if(input->getDimensions() == 2) {
            OpenCLImageAccess2D::pointer access = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            OpenCLImageAccess2D::pointer access2 = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *access->get());
            kernel.setArg(1, *access2->get());
            globalSize = cl::NDRange(output->getWidth(), output->getHeight());
        } else {
            OpenCLImageAccess3D::pointer access = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            OpenCLImageAccess3D::pointer access2 = output->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *access->get());
            kernel.setArg(1, *access2->get());
            globalSize = cl::NDRange(output->getWidth(), output->getHeight(), output->getDepth());
        }
        kernel.setArg(2, (uchar)mLabel);

        cl::CommandQueue queue = device->getCommandQueue();
        queue.enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );
    }
}

void BinaryThresholding::waitToFinish() {
    OpenCLDevice::pointer device = OpenCLDevice::pointer(getMainDevice());
    device->getCommandQueue().finish();
}

} // end namespace fast
