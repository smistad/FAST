#include "GaussianSmoothingFilter.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
using namespace fast;

void GaussianSmoothingFilter::setInput(ImageData::pointer input) {
    mInput = input;
    mIsModified = true;
    addParent(input);
    if(input->isDynamicData()) {
        mTempOutput = DynamicImage::New();
    } else {
        mTempOutput = Image::New();
        input->retain(mDevice);
    }
    mOutput = mTempOutput;
}


void GaussianSmoothingFilter::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
    mIsModified = true;
    mRecreateMask = true;
}

void GaussianSmoothingFilter::setMaskSize(unsigned char maskSize) {
    if(maskSize % 2 != 1)
        throw Exception("Mask size of GaussianSmoothingFilter must be odd.");

    mMaskSize = maskSize;
    mIsModified = true;
    mRecreateMask = true;
}

void GaussianSmoothingFilter::setStandardDeviation(float stdDev) {
    if(stdDev <= 0)
        throw Exception("Standard deviation of GaussianSmoothingFilter can't be less than 0.");

    mStdDev = stdDev;
    mIsModified = true;
    mRecreateMask = true;
}

ImageData::pointer GaussianSmoothingFilter::getOutput() {
    if(!mInput.isValid()) {
        throw Exception("Must call setInput before getOutput in GaussianSmoothingFilter");
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

GaussianSmoothingFilter::GaussianSmoothingFilter() {
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
    mStdDev = 1.0f;
    mMaskSize = 3;
    mIsModified = true;
    mRecreateMask = true;
}

GaussianSmoothingFilter::~GaussianSmoothingFilter() {
    delete[] mMask;
}

// TODO have to set mRecreateMask to true if input change dimension
void GaussianSmoothingFilter::createMask(Image::pointer input) {
    if(!mRecreateMask)
        return;

    unsigned char halfSize = (mMaskSize-1)/2;
    float sum = 0.0f;

    if(input->getDimensions() == 2) {
        mMask = new float[mMaskSize*mMaskSize];

        for(int x = -halfSize; x <= halfSize; x++) {
        for(int y = -halfSize; y <= halfSize; y++) {
            float value = exp(-(float)(x*x+y*y)/(2.0f*mStdDev*mStdDev));
            mMask[x+halfSize+(y+halfSize)*mMaskSize] = value;
            sum += value;
        }}

        for(int i = 0; i < mMaskSize*mMaskSize; i++)
            mMask[i] /= sum;
    } else if(input->getDimensions() == 3) {
         mMask = new float[mMaskSize*mMaskSize*mMaskSize];

        for(int x = -halfSize; x <= halfSize; x++) {
        for(int y = -halfSize; y <= halfSize; y++) {
        for(int z = -halfSize; z <= halfSize; z++) {
            float value = exp(-(float)(x*x+y*y+z*z)/(2.0f*mStdDev*mStdDev));
            mMask[x+halfSize+(y+halfSize)*mMaskSize+(z+halfSize)*mMaskSize*mMaskSize] = value;
            sum += value;
        }}}

        for(int i = 0; i < mMaskSize*mMaskSize*mMaskSize; i++)
            mMask[i] /= sum;
    }

    if(!mDevice->isHost()) {
        OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);
        unsigned int bufferSize = input->getDimensions() == 2 ? mMaskSize*mMaskSize : mMaskSize*mMaskSize*mMaskSize;
        mCLMask = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*bufferSize,
                mMask
        );
    }

    mRecreateMask = false;
}

void GaussianSmoothingFilter::execute() {

    Image::pointer input;
    if(!mInput.isValid()) {
        throw Exception("No input supplied to GaussianSmoothingFilter");
    }
    if(!mOutput.lock().isValid()) {
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
    if(input->getDimensions() == 2) {
        output->create2DImage(input->getWidth(),
            input->getHeight(),
            input->getDataType(),
            input->getNrOfComponents(),
            mDevice);
    } else {
         output->create3DImage(input->getWidth(),
            input->getHeight(),
            input->getDepth(),
            input->getDataType(),
            input->getNrOfComponents(),
            mDevice);
    }


    createMask(input);

    if(mDevice->isHost()) {

        // TODO: create host code

    } else {
        OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);

        std::string buildOptions = "";
        if(input->getDataType() == TYPE_FLOAT) {
            buildOptions = "-DTYPE_FLOAT";
        } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
            buildOptions = "-DTYPE_INT";
        } else {
            buildOptions = "-DTYPE_UINT";
        }
        int programNr;
        cl::NDRange globalSize;
        cl::Kernel kernel;
        if(input->getDimensions() == 2) {
            // TODO: unnecessary to compile each time
            programNr = device->createProgramFromSource(std::string(FAST_ROOT_DIR) + "Algorithms/GaussianSmoothingFilter2D.cl", buildOptions);
            kernel = cl::Kernel(device->getProgram(programNr), "gaussianSmoothing");
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());

            OpenCLImageAccess2D inputAccess = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            OpenCLImageAccess2D outputAccess = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *inputAccess.get());
            kernel.setArg(2, *outputAccess.get());
        } else {
            // TODO: unnecessary to compile each time
            programNr = device->createProgramFromSource(std::string(FAST_ROOT_DIR) + "Algorithms/GaussianSmoothingFilter3D.cl", buildOptions);
            kernel = cl::Kernel(device->getProgram(programNr), "gaussianSmoothing");
            globalSize = cl::NDRange(input->getWidth(),input->getHeight(),input->getDepth());

            OpenCLImageAccess3D inputAccess = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            OpenCLImageAccess3D outputAccess = output->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
            kernel.setArg(0, *inputAccess.get());
            kernel.setArg(2, *outputAccess.get());
        }

        kernel.setArg(1, mCLMask);
        kernel.setArg(3, mMaskSize);

        device->getCommandQueue().enqueueNDRangeKernel(
                kernel,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );
    }

    if(!mInput->isDynamicData())
        mInput->release(mDevice);

    // Update the timestamp of the output data
    output->updateModifiedTimestamp();
}

void GaussianSmoothingFilter::waitToFinish() {
    if(!mDevice->isHost()) {
        OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);
        device->getCommandQueue().finish();
    }
}
