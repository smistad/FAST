#include "FAST/Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

void GaussianSmoothingFilter::setMaskSize(unsigned char maskSize) {
    if(maskSize <= 0)
        throw Exception("Mask size of GaussianSmoothingFilter can't be less than 0.");
    if(maskSize % 2 != 1)
        throw Exception("Mask size of GaussianSmoothingFilter must be odd.");

    mMaskSize = maskSize;
    mIsModified = true;
    mRecreateMask = true;
}

void GaussianSmoothingFilter::setOutputType(DataType type) {
    mOutputType = type;
    mOutputTypeSet = true;
    mIsModified = true;
}

void GaussianSmoothingFilter::setStandardDeviation(float stdDev) {
    if(stdDev <= 0)
        throw Exception("Standard deviation of GaussianSmoothingFilter can't be less than 0.");

    mStdDev = stdDev;
    mIsModified = true;
    mRecreateMask = true;
}

GaussianSmoothingFilter::GaussianSmoothingFilter() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    mStdDev = 1.0f;
    mMaskSize = 3;
    mIsModified = true;
    mRecreateMask = true;
    mDimensionCLCodeCompiledFor = 0;
    mMask = NULL;
    mOutputTypeSet = false;
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

    ExecutionDevice::pointer device = getMainDevice();
    if(!device->isHost()) {
        OpenCLDevice::pointer clDevice = device;
        unsigned int bufferSize = input->getDimensions() == 2 ? mMaskSize*mMaskSize : mMaskSize*mMaskSize*mMaskSize;
        mCLMask = cl::Buffer(
                clDevice->getContext(),
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

    OpenCLDevice::pointer device = getMainDevice();
    std::string buildOptions = "";
    const bool writingTo3DTextures = device->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
    if(!writingTo3DTextures) {
    switch(mOutputType) {
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
    }
    std::string filename;
    if(input->getDimensions() == 2) {
        filename = "Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter2D.cl";
    } else {
        filename = "Algorithms/GaussianSmoothingFilter/GaussianSmoothingFilter3D.cl";
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
    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ_WRITE);

    T * inputData = (T*)inputAccess->get();
    T * outputData = (T*)outputAccess->get();

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
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer output = getStaticOutputData<Image>(0);


    // Initialize output image
    ExecutionDevice::pointer device = getMainDevice();
    if(mOutputTypeSet) {
        if(input->getDimensions() == 2) {
            output->create2DImage(
                    input->getWidth(),
                    input->getHeight(),
                    mOutputType,
                    input->getNrOfComponents(),
                    device
            );
        } else {
             output->create3DImage(
                    input->getWidth(),
                    input->getHeight(),
                    input->getDepth(),
                    mOutputType,
                    input->getNrOfComponents(),
                    device
            );
        }
    } else {
        output->createFromImage(input, device);
    }
    mOutputType = output->getDataType();

    createMask(input);

    if(device->isHost()) {
        switch(input->getDataType()) {
            fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output, mMask, mMaskSize));
        }
    } else {
        OpenCLDevice::pointer clDevice = device;

        recompileOpenCLCode(input);
        cl::NDRange globalSize;
        if(input->getDimensions() == 2) {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());

            OpenCLImageAccess2D::pointer inputAccess = input->getOpenCLImageAccess2D(ACCESS_READ, device);
            OpenCLImageAccess2D::pointer outputAccess = output->getOpenCLImageAccess2D(ACCESS_READ_WRITE, device);
            mKernel.setArg(0, *inputAccess->get());
            mKernel.setArg(2, *outputAccess->get());
        } else {
            globalSize = cl::NDRange(input->getWidth(),input->getHeight(),input->getDepth());

            const bool writingTo3DTextures = clDevice->getDevice().getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_3d_image_writes") != std::string::npos;
            OpenCLImageAccess3D::pointer inputAccess = input->getOpenCLImageAccess3D(ACCESS_READ, device);
            mKernel.setArg(0, *inputAccess->get());
            if(writingTo3DTextures) {
                OpenCLImageAccess3D::pointer outputAccess = output->getOpenCLImageAccess3D(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess->get());
            } else {
                OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess->get());
            }
        }

        mKernel.setArg(1, mCLMask);
        mKernel.setArg(3, mMaskSize);

        clDevice->getCommandQueue().enqueueNDRangeKernel(
                mKernel,
                cl::NullRange,
                globalSize,
                cl::NullRange
        );
    }
}

void GaussianSmoothingFilter::waitToFinish() {
    if(!getMainDevice()->isHost()) {
        OpenCLDevice::pointer device = getMainDevice();
        device->getCommandQueue().finish();
    }
}
