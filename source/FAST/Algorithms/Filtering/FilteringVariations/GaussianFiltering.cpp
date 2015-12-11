#include "FAST/Algorithms/Filtering/FilteringVariations/GaussianFiltering.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

void GaussianFiltering::setStandardDeviation(float stdDev) {
if (stdDev <= 0)
throw Exception("Standard deviation of GaussianFiltering can't be less than 0.");

mStdDev = stdDev;
mMaskDefaultSize = (char)(ceil(2 * mStdDev) * 2 + 1);
mIsModified = true;
mRecreateMask = true;
}


GaussianFiltering::GaussianFiltering() : Filtering() {
    mStdDev = 0.5f;
    mMaskDefaultSize = (char)(ceil(2 * mStdDev) * 2 + 1);
    /*
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/Filtering/SpatialFilter2D.cl", "2D"); //TODO fix OpenCL
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/Filtering/SpatialFilter3D.cl", "3D");
    mMaskSize = -1;
    mIsModified = true;
    mRecreateMask = true;
    mDimensionCLCodeCompiledFor = 0;
    mMask = NULL;
    mOutputTypeSet = false;
    */
}

GaussianFiltering::~GaussianFiltering() {
    delete[] mMask;
}

float GaussianFiltering::createMaskValue(int x, int y, int z){
    if (z==0)
        return exp(-(float)(x*x + y*y) / (2.0f*mStdDev*mStdDev));
    else
        return exp(-(float)(x*x + y*y + z*z) / (2.0f*mStdDev*mStdDev));
}
/*
// TODO have to set mRecreateMask to true if input change dimension
void GaussianFiltering::createMask(Image::pointer input, uchar maskSize) {
    if (!mRecreateMask)
        return;

    unsigned char halfSize = (maskSize - 1) / 2;
    float sum = 0.0f;

    if (input->getDimensions() == 2){
        mMask = new float[maskSize*maskSize];
        for (int x = -halfSize; x <= halfSize; x++){
            for (int y = -halfSize; y <= halfSize; y++){
                //float value = 1; //or get mask value
                float value = exp(-(float)(x*x + y*y) / (2.0f*mStdDev*mStdDev));
                mMask[x + halfSize + (y + halfSize)*maskSize] = value; //can also just have a counter
                sum += value;
            }
        }
        for (int i = 0; i < maskSize*maskSize; ++i) //starte den her riktig på 0 eller 1?
            mMask[i] /= sum;
    } else if (input->getDimensions() == 3){
        mMask = new float[maskSize*maskSize];
        for (int x = -halfSize; x <= halfSize; x++){
            for (int y = -halfSize; y <= halfSize; y++){
                for (int z = -halfSize; z <= halfSize; z++){
                    //float value = 1;
                    float value = exp(-(float)(x*x + y*y + z*z) / (2.0f*mStdDev*mStdDev));
                    mMask[x + halfSize + (y + halfSize)*maskSize + (z + halfSize)*maskSize*maskSize] = value; //can also just have a counter
                    sum += value;
                }
            }
        }
        for (int i = 0; i < maskSize*maskSize*maskSize; ++i) //starte den her riktig på 0 eller 1?
            mMask[i] /= sum;
    }

    ExecutionDevice::pointer device = getMainDevice();
    if (!device->isHost()) {
        OpenCLDevice::pointer clDevice = device;
        unsigned int bufferSize = input->getDimensions() == 2 ? maskSize*maskSize : maskSize*maskSize*maskSize;
        mCLMask = cl::Buffer(
            clDevice->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(float)*bufferSize,
            mMask
            );
    }

    mRecreateMask = false;
}
*/

bool GaussianFiltering::isSeparable(){
    return true;
}

float * GaussianFiltering::getSeparable(int dir){
    //direction does not matter
    //std::cout << "mMaskSize sep " << mMaskSize << std::endl;
    //std::cout << "mMaskSize sep INT " << int(mMaskSize) << std::endl;
    //float mask[int(mMaskSize)] = {};
    int maskSize = int(mMaskSize);
    float * mask = new float[maskSize];
    int halfSize = (maskSize - 1) / 2;
    float expDiv = (2.0f*mStdDev*mStdDev);
    float sqrtPart = 1;// (1 / (sqrt(2 * M_PI)*mStdDev));
    float sum = 0.0f;
    for (int i = 0; i < maskSize; i++){
        int x = i - halfSize;
        float value = sqrtPart*exp(-(float)(x*x) / expDiv);
        mask[i] = value;
        sum += value;
    }
    //std::cout << "separable sum: " << sum << std::endl;
    //ToDO average it?
    for (int i = 0; i < maskSize; i++){
        mask[i] /= sum;
    }
    return mask;
}

/* //COULD BE KEPT
void GaussianFiltering::recompileOpenCLCode(Image::pointer input) {
    // Check if there is a need to recompile OpenCL code
    if (input->getDimensions() == mDimensionCLCodeCompiledFor &&
        input->getDataType() == mTypeCLCodeCompiledFor)
        return;

    OpenCLDevice::pointer device = getMainDevice();
    std::string buildOptions = "";
    if (!device->isWritingTo3DTexturesSupported()) {
        switch (mOutputType) {
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
    cl::Program program;
    if (input->getDimensions() == 2) {
        program = getOpenCLProgram(device, "2D", buildOptions);
    }
    else {
        program = getOpenCLProgram(device, "3D", buildOptions);
    }
    mKernel = cl::Kernel(program, "SpatialFilter");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
} */

/*
template <class T>
void executeAlgorithmOnHost(Image::pointer input, Image::pointer output, float * mask, unsigned char maskSize) {
    // TODO: this method currently only processes the first component
    unsigned int nrOfComponents = input->getNrOfComponents();
    ImageAccess::pointer inputAccess = input->getImageAccess(ACCESS_READ);
    ImageAccess::pointer outputAccess = output->getImageAccess(ACCESS_READ_WRITE);

    T * inputData = (T*)inputAccess->get();
    T * outputData = (T*)outputAccess->get();

    const unsigned char halfSize = (maskSize - 1) / 2;
    unsigned int width = input->getWidth();
    unsigned int height = input->getHeight();
    if (input->getDimensions() == 3) {
        unsigned int depth = input->getDepth();
        for (unsigned int z = 0; z < depth; z++) {
            for (unsigned int y = 0; y < height; y++) {
                for (unsigned int x = 0; x < width; x++) {

                    if (x < halfSize || x >= width - halfSize ||
                        y < halfSize || y >= height - halfSize ||
                        z < halfSize || z >= depth - halfSize) {
                        // on border only copy values
                        outputData[x*nrOfComponents + y*nrOfComponents*width + z*nrOfComponents*width*height] = inputData[x*nrOfComponents + y*nrOfComponents*width + z*nrOfComponents*width*height];
                        continue;
                    }

                    double sum = 0.0;
                    for (int c = -halfSize; c <= halfSize; c++) {
                        for (int b = -halfSize; b <= halfSize; b++) {
                            for (int a = -halfSize; a <= halfSize; a++) {
                                sum += mask[a + halfSize + (b + halfSize)*maskSize + (c + halfSize)*maskSize*maskSize] *
                                    inputData[(x + a)*nrOfComponents + (y + b)*nrOfComponents*width + (z + c)*nrOfComponents*width*height];
                            }
                        }
                    }
                    outputData[x*nrOfComponents + y*nrOfComponents*width + z*nrOfComponents*width*height] = (T)sum;
                }
            }
        }
    }
    else {
        for (unsigned int y = halfSize; y < height - halfSize; y++) {
            for (unsigned int x = halfSize; x < width - halfSize; x++) {

                if (x < halfSize || x >= width - halfSize ||
                    y < halfSize || y >= height - halfSize) {
                    // on border only copy values
                    outputData[x*nrOfComponents + y*nrOfComponents*width] = inputData[x*nrOfComponents + y*nrOfComponents*width];
                    continue;
                }

                double sum = 0.0;
                for (int b = -halfSize; b <= halfSize; b++) {
                    for (int a = -halfSize; a <= halfSize; a++) {
                        sum += mask[a + halfSize + (b + halfSize)*maskSize] *
                            inputData[(x + a)*nrOfComponents + (y + b)*nrOfComponents*width];
                    }
                }
                outputData[x*nrOfComponents + y*nrOfComponents*width] = (T)sum;
            }
        }
    }
}
*/

/*
void GaussianFiltering::execute() {
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer output = getStaticOutputData<Image>(0);

    char maskSize = mMaskSize;
    if (maskSize <= 0) // If mask size is not set calculate it instead
        maskSize = mMaskDefaultSize; //Default to 3 // ceil(2 * mStdDev) * 2 + 1;

    if (maskSize > 19)
        maskSize = 19;

    // Initialize output image
    ExecutionDevice::pointer device = getMainDevice();
    if (mOutputTypeSet) {
        output->create(input->getSize(), mOutputType, input->getNrOfComponents());
        output->setSpacing(input->getSpacing());
    }
    else {
        output->createFromImage(input);
    }
    mOutputType = output->getDataType();
    SceneGraph::setParentNode(output, input);

    createMask(input, maskSize);

    if (device->isHost()) {
        switch (input->getDataType()) {
            //fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output, mMask, maskSize));
        }
    }
    else {
        OpenCLDevice::pointer clDevice = device;

        recompileOpenCLCode(input);
        cl::NDRange globalSize;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        if (input->getDimensions() == 2) {
            globalSize = cl::NDRange(input->getWidth(), input->getHeight());

            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            mKernel.setArg(0, *inputAccess->get2DImage());
            mKernel.setArg(2, *outputAccess->get2DImage());
        }
        else {
            globalSize = cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth());

            mKernel.setArg(0, *inputAccess->get3DImage());
            if (clDevice->isWritingTo3DTexturesSupported()) {
                OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess->get3DImage());
            }
            else {
                OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess->get());
            }
        }

        mKernel.setArg(1, mCLMask);
        mKernel.setArg(3, maskSize);

        clDevice->getCommandQueue().enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            globalSize,
            cl::NullRange
            );
    }
}
*/

/*
void GaussianFiltering::waitToFinish() {
    if (!getMainDevice()->isHost()) {
        OpenCLDevice::pointer device = getMainDevice();
        device->getCommandQueue().finish();
    }
}
*/
