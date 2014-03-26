#include "GaussianSmoothingFilter2D.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
using namespace fast;

void GaussianSmoothingFilter2D::setInput(Image2D::pointer input) {
    mStaticInput = input;
    mIsModified = true;
    OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);
    input->update(); // Need to run update to get the width and height, could maybe be moved into getWidth?
    cl::Image2D* clImage = new cl::Image2D(
            device->getContext(),
            CL_MEM_WRITE_ONLY,
            cl::ImageFormat(CL_R, CL_FLOAT),
            input->getWidth(), input->getHeight()
            );
    mOutput.lock()->setOpenCLImage(clImage,device);
    addParent(input);
}

void GaussianSmoothingFilter2D::setInput(Image2Dt::pointer input) {
    mDynamicInput = input;
    mIsModified = true;
    // TODO finish
    addParent(input);
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
    if(mTempOutput.isValid()) {
        mTempOutput->addParent(mPtr.lock());

        Image2D::pointer newSmartPtr;
        newSmartPtr.swap(mTempOutput);

        return newSmartPtr;
    } else {
        return mOutput.lock();
    }
}

GaussianSmoothingFilter2D::GaussianSmoothingFilter2D() {
    mTempOutput = Image2D::New();
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();

    mOutput = mTempOutput;
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
    if(!mStaticInput.isValid() && !mDynamicInput.isValid()) {
        throw Exception("No input supplied to GaussianSmoothingFilter2D");
    } else if(mStaticInput.isValid()) {
        input = mStaticInput;
    } else {
        input = mDynamicInput->getNextFrame();
    }

    Image2D::pointer output = mOutput.lock();
    if(output == NULL) {
        // output object is no longer valid
        return;
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
}
