#include "GaussianSmoothingFilter.hpp"
#include "Exception.hpp"
#include "DeviceManager.hpp"
#include "Image.hpp"
#include "DynamicImage.hpp"
using namespace fast;

void GaussianSmoothingFilter::setInput(ImageData::pointer input) {
    mInput = input;
    mIsModified = true;
    setParent(input);
    if(input->isDynamicData()) {
        mOutput = DynamicImage::New();
        DynamicImage::pointer(mOutput)->setStreamer(DynamicImage::pointer(mInput)->getStreamer());
    } else {
        mOutput = Image::New();
        input->retain(mDevice);
    }
    mOutput->setSource(mPtr.lock());
}


void GaussianSmoothingFilter::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
    mIsModified = true;
    mRecreateMask = true;
}

void GaussianSmoothingFilter::setMaskSize(unsigned char maskSize) {
    if(maskSize <= 0)
        throw Exception("Mask size of GaussianSmoothingFilter can't be less than 0.");
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
    if(!mOutput.isValid()) {
        throw Exception("Must call setInput before getOutput in GaussianSmoothingFilter");
    }
    return mOutput;
}

GaussianSmoothingFilter::GaussianSmoothingFilter() {
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
    mStdDev = 1.0f;
    mMaskSize = 3;
    mIsModified = true;
    mRecreateMask = true;
    mDimensionCLCodeCompiledFor = 0;
    mMask = NULL;
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

void GaussianSmoothingFilter::recompileOpenCLCode(Image::pointer input) {
    // Check if there is a need to recompile OpenCL code
    if(input->getDimensions() == mDimensionCLCodeCompiledFor &&
            input->getDataType() == mTypeCLCodeCompiledFor)
        return;

    OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);
    std::string buildOptions = "";
    if(input->getDataType() == TYPE_FLOAT) {
        buildOptions = "-DTYPE_FLOAT";
    } else if(input->getDataType() == TYPE_INT8 || input->getDataType() == TYPE_INT16) {
        buildOptions = "-DTYPE_INT";
    } else {
        buildOptions = "-DTYPE_UINT";
    }
    switch(input->getDataType()) {
        case TYPE_FLOAT:
            buildOptions += " -DTYPE=float";
            break;
        case TYPE_INT8:
            buildOptions += " -DTYPE=char";
            break;
        case TYPE_UINT8:
            buildOptions += " -DTYPE=uchar";
            break;
        case TYPE_INT16:
            buildOptions += " -DTYPE=short";
            break;
        case TYPE_UINT16:
            buildOptions += " -DTYPE=ushort";
            break;
        }
    std::string filename;
    if(input->getDimensions() == 2) {
        filename = "Algorithms/GaussianSmoothingFilter2D.cl";
    } else {
        filename = "Algorithms/GaussianSmoothingFilter3D.cl";
    }
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + filename, buildOptions);
    mKernel = cl::Kernel(device->getProgram(programNr), "gaussianSmoothing");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
}

template <class T>
void executeAlgorithmOnHost(Image::pointer input, Image::pointer output, float * mask, unsigned char maskSize) {
    // TODO: this method currently only processes the first component
    unsigned int nrOfComponents = input->getNrOfComponents();
    ImageAccess inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess outputAccess = output->getImageAccess(ACCESS_READ_WRITE);

    T * inputData = (T*)inputAccess.get();
    T * outputData = (T*)outputAccess.get();

    const unsigned char halfSize = (maskSize-1)/2;
    unsigned int width = input->getWidth();
    unsigned int height = input->getHeight();
    if(input->getDimensions() == 3) {
        unsigned int depth = input->getDepth();
        for(unsigned int z = 0; z < depth; z++) {
        for(unsigned int y = 0; y < height; y++) {
        for(unsigned int x = 0; x < width; x++) {

            if(x < halfSize || x >= width-halfSize ||
            y < halfSize || y >= height-halfSize ||
            z < halfSize || z >= depth-halfSize) {
                // on border only copy values
                outputData[x*nrOfComponents+y*nrOfComponents*width+z*nrOfComponents*width*height] = inputData[x*nrOfComponents+y*nrOfComponents*width+z*nrOfComponents*width*height];
                continue;
            }

            double sum = 0.0;
            for(int c = -halfSize; c <= halfSize; c++) {
            for(int b = -halfSize; b <= halfSize; b++) {
            for(int a = -halfSize; a <= halfSize; a++) {
                sum += mask[a+halfSize+(b+halfSize)*maskSize+(c+halfSize)*maskSize*maskSize]*
                        inputData[(x+a)*nrOfComponents+(y+b)*nrOfComponents*width+(z+c)*nrOfComponents*width*height];
            }}}
            outputData[x*nrOfComponents+y*nrOfComponents*width+z*nrOfComponents*width*height] = (T)sum;
        }}}
    } else {
        for(unsigned int y = halfSize; y < height-halfSize; y++) {
        for(unsigned int x = halfSize; x < width-halfSize; x++) {

            if(x < halfSize || x >= width-halfSize ||
            y < halfSize || y >= height-halfSize) {
                // on border only copy values
                outputData[x*nrOfComponents+y*nrOfComponents*width] = inputData[x*nrOfComponents+y*nrOfComponents*width];
                continue;
            }

            double sum = 0.0;
            for(int b = -halfSize; b <= halfSize; b++) {
            for(int a = -halfSize; a <= halfSize; a++) {
                sum += mask[a+halfSize+(b+halfSize)*maskSize]*
                        inputData[(x+a)*nrOfComponents+(y+b)*nrOfComponents*width];
            }}
            outputData[x*nrOfComponents+y*nrOfComponents*width] = (T)sum;
        }}
    }
}

void GaussianSmoothingFilter::execute() {

    Image::pointer input;
    if(!mInput.isValid()) {
        throw Exception("No input supplied to GaussianSmoothingFilter");
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
    output->createFromImage(input, mDevice);

    createMask(input);

    if(mDevice->isHost()) {
        switch(input->getDataType()) {
            fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output, mMask, mMaskSize));
        }
    } else {
        OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);

        recompileOpenCLCode(input);
        cl::NDRange globalSize;
        if(input->getDimensions() == 2) {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());

            OpenCLImageAccess2D inputAccess = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            OpenCLImageAccess2D outputAccess = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            mKernel.setArg(0, *inputAccess.get());
            mKernel.setArg(2, *outputAccess.get());
        } else {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight(),input->getDepth());

            const bool writingTo3DTextures = device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
            OpenCLImageAccess3D inputAccess = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            mKernel.setArg(0, *inputAccess.get());
            if(writingTo3DTextures) {
                OpenCLImageAccess3D outputAccess = output->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess.get());
            } else {
                OpenCLBufferAccess outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess.get());
            }
        }

        mKernel.setArg(1, mCLMask);
        mKernel.setArg(3, mMaskSize);

        device->getCommandQueue().enqueueNDRangeKernel(
                mKernel,
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
