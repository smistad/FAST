#include "GaussianSmoothingFilter2D.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "Image2D.hpp"
#include "Image2Dt.hpp"
using namespace fast;

void GaussianSmoothingFilter2D::setInput(ImageData::pointer input) {
    mInput = input;
    mIsModified = true;
    addParent(input);
    if(input->isDynamicData()) {
        mTempOutput = Image2Dt::New();
    } else {
        mTempOutput = Image2D::New();
        input->retain(mDevice);
    }
    mOutput = mTempOutput;
}


void GaussianSmoothingFilter2D::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
    mIsModified = true;
}

void GaussianSmoothingFilter2D::setMaskSize(unsigned char maskSize) {
    if(maskSize % 2 != 1)
        throw Exception("Mask size of GaussianSmoothingFilter must be odd.");

    mMaskSize = maskSize;
    mIsModified = true;
}

void GaussianSmoothingFilter2D::setStandardDeviation(float stdDev) {
    if(stdDev <= 0)
        throw Exception("Standard deviation of GaussianSmoothingFilter can't be less than 0.");

    mStdDev = stdDev;
    mIsModified = true;
}

ImageData::pointer GaussianSmoothingFilter2D::getOutput() {
    if(!mInput.isValid()) {
        throw Exception("Must call setInput before getOutput in GaussianSmoothingFilter2D");
    }
    if(mTempOutput.isValid()) {
        mTempOutput->setParent(mPtr.lock());

        ImageData::pointer newSmartPtr;
        newSmartPtr.swap(mTempOutput);

        return newSmartPtr;
    } else {
        return mOutput.lock();
    }
}

GaussianSmoothingFilter2D::GaussianSmoothingFilter2D() {
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
    mStdDev = 1.0f;
    mMaskSize = 3;
    mIsModified = true;
}

float * GaussianSmoothingFilter2D::createMask() {
    float * mask = new float[mMaskSize*mMaskSize];

    unsigned char halfSize = (mMaskSize-1)/2;
    float sum = 0.0f;

    for(int x = -halfSize; x <= halfSize; x++) {
    for(int y = -halfSize; y <= halfSize; y++) {
        float value = exp(-(float)(x*x+y*y)/(2.0f*mStdDev*mStdDev));
        mask[x+halfSize+(y+halfSize)*mMaskSize] = value;
        sum += value;
    }}

    for(int i = 0; i < mMaskSize*mMaskSize; i++)
        mask[i] /= sum;

    return mask;
}

void GaussianSmoothingFilter2D::execute() {

    Image2D::pointer input;
    if(!mInput.isValid()) {
        throw Exception("No input supplied to GaussianSmoothingFilter2D");
    }
    if(mInput->isDynamicData()) {
        input = Image2Dt::pointer(mInput)->getNextFrame();
        std::cout << "processing a new frame" << std::endl;
    } else {
        input = mInput;
    }

    Image2D::pointer output;
    if(mInput->isDynamicData()) {
        output = Image2D::New();

        // Initialize output image
        output->createImage(input->getWidth(),
            input->getHeight(),
            input->getDataType(),
            input->getNrOfComponents(),
            mDevice);

        Image2Dt::pointer(mOutput)->addFrame(output);
    } else {
        output = Image2D::pointer(mOutput);


        if(output == NULL) {
            // output object is no longer valid
            return;
        }

        // Initialize output image
        output->createImage(input->getWidth(),
                input->getHeight(),
                input->getDataType(),
                input->getNrOfComponents(),
                mDevice);

    }

    if(mDevice->isHost()) {

        // TODO: create host code

    } else {
        OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);

        float * mask = createMask();
        cl::Buffer clMask = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*mMaskSize*mMaskSize,
                mask
        );
        delete[] mask;

        int programNr = device->createProgramFromSource(std::string(FAST_ROOT_DIR) + "Algorithms/GaussianSmoothingFilter2D.cl");
        cl::Kernel kernel(device->getProgram(programNr), "gaussianSmoothing");

        OpenCLImageAccess2D inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess2D outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        kernel.setArg(0, *inputAccess.get());
        kernel.setArg(1, clMask);
        kernel.setArg(2, *outputAccess.get());
        kernel.setArg(3, mMaskSize);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(),input->getHeight()),
                cl::NullRange
        );
    }

    if(!mInput->isDynamicData())
        mInput->release(mDevice);

    // Update the timestamp of the output data
    output->updateModifiedTimestamp();
}
