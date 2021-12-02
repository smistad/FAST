#include "FAST/Algorithms/GaussianSmoothing/GaussianSmoothing.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

void GaussianSmoothing::setMaskSize(unsigned char maskSize) {
    if(maskSize <= 0)
        throw Exception("Mask size of GaussianSmoothing can't be less than 0.");
    if(maskSize % 2 != 1)
        throw Exception("Mask size of GaussianSmoothing must be odd.");

    mMaskSize = maskSize;
    mIsModified = true;
    mRecreateMask = true;
}

void GaussianSmoothing::setOutputType(DataType type) {
    mOutputType = type;
    mOutputTypeSet = true;
    mIsModified = true;
}

void GaussianSmoothing::setStandardDeviation(float stdDev) {
    if(stdDev <= 0)
        throw Exception("Standard deviation of GaussianSmoothing can't be less than 0.");

    mStdDev = stdDev;
    mIsModified = true;
    mRecreateMask = true;
}

GaussianSmoothing::GaussianSmoothing(float stdDev, uchar maskSize) {
    createInputPort(0, "Image");
    createOutputPort(0, "Image");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/GaussianSmoothing/GaussianSmoothing2D.cl", "2D");
    createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/GaussianSmoothing/GaussianSmoothing3D.cl", "3D");
    createFloatAttribute("stdev", "Standard deviation", "Standard deviation", stdDev);
    mIsModified = true;
    mRecreateMask = true;
    mDimensionCLCodeCompiledFor = 0;
    mMask = NULL;
    mOutputTypeSet = false;
    setStandardDeviation(stdDev);
    if(maskSize > 0)
        setMaskSize(maskSize);
}

GaussianSmoothing::~GaussianSmoothing() {
}

// TODO have to set mRecreateMask to true if input change dimension
void GaussianSmoothing::createMask(Image::pointer input, uchar maskSize, bool useSeperableFilter) {
    if(!mRecreateMask)
        return;

    unsigned char halfSize = (maskSize-1)/2;
    float sum = 0.0f;

    if(input->getDimensions() == 2) {
        mMask = std::make_unique<float[]>(maskSize*maskSize);

        for(int x = -halfSize; x <= halfSize; x++) {
        for(int y = -halfSize; y <= halfSize; y++) {
            float value = exp(-(float)(x*x+y*y)/(2.0f*mStdDev*mStdDev));
            mMask[x+halfSize+(y+halfSize)*maskSize] = value;
            sum += value;
        }}

        for(int i = 0; i < maskSize*maskSize; ++i)
            mMask[i] /= sum;
    } else if(input->getDimensions() == 3) {
        // Use separable filtering for 3D
        if(useSeperableFilter) {
            mMask = std::make_unique<float[]>(maskSize);

            for(int x = -halfSize; x <= halfSize; x++) {
                float value = exp(-(float)(x*x)/(2.0f*mStdDev*mStdDev));
                mMask[x+halfSize] = value;
                sum += value;
            }

            for(int i = 0; i < maskSize; ++i)
                mMask[i] /= sum;
        } else {
            mMask = std::make_unique<float[]>(maskSize*maskSize*maskSize);

            for(int x = -halfSize; x <= halfSize; x++) {
            for(int y = -halfSize; y <= halfSize; y++) {
            for(int z = -halfSize; z <= halfSize; z++) {
                float value = exp(-(float)(x*x+y*y+z*z)/(2.0f*mStdDev*mStdDev));
                mMask[x+halfSize+(y+halfSize)*maskSize+(z+halfSize)*maskSize*maskSize] = value;
                sum += value;
            }}}

            for(int i = 0; i < maskSize*maskSize*maskSize; ++i)
                mMask[i] /= sum;
        }
    }

    ExecutionDevice::pointer device = getMainDevice();
    if(!device->isHost()) {
        OpenCLDevice::pointer clDevice = std::dynamic_pointer_cast<OpenCLDevice>(device);
        uint bufferSize;
        if(useSeperableFilter) {
            bufferSize = maskSize;
        } else {
            bufferSize = input->getDimensions() == 2 ? maskSize*maskSize : maskSize*maskSize*maskSize;
        }
        mCLMask = cl::Buffer(
                clDevice->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*bufferSize,
                mMask.get()
        );
    }

    mRecreateMask = false;
}

void GaussianSmoothing::recompileOpenCLCode(Image::pointer input) {
    // Check if there is a need to recompile OpenCL code
    if(input->getDimensions() == mDimensionCLCodeCompiledFor &&
            input->getDataType() == mTypeCLCodeCompiledFor)
        return;

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    std::string buildOptions = "";
    if(!device->isWritingTo3DTexturesSupported()) {
        buildOptions = "-DTYPE=" + getCTypeAsString(mOutputType);
    }
    cl::Program program;
    if(input->getDimensions() == 2) {
        program = getOpenCLProgram(device, "2D", buildOptions);
    } else {
        program = getOpenCLProgram(device, "3D", buildOptions);
    }
    mKernel = cl::Kernel(program, "gaussianSmoothing");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
}

template <class T>
void executeAlgorithmOnHost(Image::pointer input, Image::pointer output, const float* const mask, unsigned char maskSize) {
    // TODO: this method currently only processes the first component
    unsigned int nrOfComponents = input->getNrOfChannels();
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

void GaussianSmoothing::execute() {
    auto input = getInputData<Image>(0);

    char maskSize = mMaskSize;
    if(maskSize <= 0) // If mask size is not set calculate it instead
        maskSize = ceil(2*mStdDev)*2+1;

    if(maskSize > 19)
        maskSize = 19;

    // Initialize output image
    ExecutionDevice::pointer device = getMainDevice();
    Image::pointer output;
    if(mOutputTypeSet) {
        output = Image::create(input->getSize(), mOutputType, input->getNrOfChannels());
        output->setSpacing(input->getSpacing());
    } else {
        output = Image::createFromImage(input);
    }
    mOutputType = output->getDataType();
    SceneGraph::setParentNode(output, input);


    if(device->isHost()) {
        createMask(input, maskSize, false);
        switch(input->getDataType()) {
            fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output, mMask.get(), maskSize));
        }
    } else {
        OpenCLDevice::pointer clDevice = std::static_pointer_cast<OpenCLDevice>(device);

        recompileOpenCLCode(input);

        cl::NDRange globalSize;

        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, clDevice);
        if(input->getDimensions() == 2) {
            createMask(input, maskSize, false);
            mKernel.setArg(1, mCLMask);
            mKernel.setArg(3, maskSize);
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());

            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
            mKernel.setArg(0, *inputAccess->get2DImage());
            mKernel.setArg(2, *outputAccess->get2DImage());
            clDevice->getCommandQueue().enqueueNDRangeKernel(
                    mKernel,
                    cl::NullRange,
                    globalSize,
                    cl::NullRange
            );
        } else {
            // Create an auxilliary image
            auto output2 = Image::createFromImage(output);

            globalSize = cl::NDRange(input->getWidth(),input->getHeight(),input->getDepth());

            if(clDevice->isWritingTo3DTexturesSupported()) {
                createMask(input, maskSize, true);
                mKernel.setArg(1, mCLMask);
                mKernel.setArg(3, maskSize);
                OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);
                OpenCLImageAccess::pointer outputAccess2 = output2->getOpenCLImageAccess(ACCESS_READ_WRITE, clDevice);

                cl::Image3D* image2;
                cl::Image3D* image;
                image = outputAccess->get3DImage();
                image2 = outputAccess->get3DImage();
                for(uchar direction = 0; direction < input->getDimensions(); ++direction) {
                    if(direction == 0) {
                        mKernel.setArg(0, *inputAccess->get3DImage());
                        mKernel.setArg(2, *image);
                    } else if(direction == 1) {
                        mKernel.setArg(0, *image);
                        mKernel.setArg(2, *image2);
                    } else {
                        mKernel.setArg(0, *image2);
                        mKernel.setArg(2, *image);
                    }
                    mKernel.setArg(4, direction);
                    clDevice->getCommandQueue().enqueueNDRangeKernel(
                            mKernel,
                            cl::NullRange,
                            globalSize,
                            cl::NullRange
                    );
                }
            } else {
                createMask(input, maskSize, false);
                mKernel.setArg(1, mCLMask);
                mKernel.setArg(3, maskSize);
                OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, clDevice);
                mKernel.setArg(0, *inputAccess->get3DImage());
                mKernel.setArg(2, *outputAccess->get());
                clDevice->getCommandQueue().enqueueNDRangeKernel(
                        mKernel,
                        cl::NullRange,
                        globalSize,
                        cl::NullRange
                );
            }


        }
    }
    addOutputData(0, output);
}

void GaussianSmoothing::waitToFinish() {
    if(!getMainDevice()->isHost()) {
        OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
        device->getCommandQueue().finish();
    }
}

void GaussianSmoothing::loadAttributes() {
    setStandardDeviation(getFloatAttribute("stdev"));
}
