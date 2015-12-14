#include "FAST/Algorithms/Filtering/Filtering.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"

using namespace fast;

void Filtering::setMaskSize(unsigned char maskSize) {
    // getNameOfClass() (works with FAST_OBJECT defined classes)
    if (maskSize <= 0) // unsigned + check below ZERO? lol
        throw Exception("Mask size of Filtering can't be less than 0.");
    if (maskSize % 2 != 1)
        throw Exception("Mask size of Filtering must be odd.");

    mMaskSize = maskSize;
    mIsModified = true;
    mRecreateMask = true;
}

void Filtering::setOutputType(DataType type) {
    mOutputType = type;
    mOutputTypeSet = true;
    mIsModified = true;
}

/*
void Filtering::setStandardDeviation(float stdDev) {
if (stdDev <= 0)
throw Exception("Standard deviation of GaussianSmoothingFilter can't be less than 0.");

mStdDev = stdDev;
mIsModified = true;
mRecreateMask = true;
}
*/

void Filtering::setConvRunType(int runType) {
    mConvRunType = runType;
    mIsModified = true;
}


Filtering::Filtering() {
    createInputPort<Image>(0);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/Filtering/SpatialFilter2D.cl", "2D"); //TODO fix OpenCL
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/Filtering/SpatialFilter3D.cl", "3D");
    //mStdDev = 0.5f;
    mMaskSize = -1;
    mMaskDefaultSize = 3;
    mIsModified = true;
    mRecreateMask = true;
    mDimensionCLCodeCompiledFor = 0; //necessary?
    mMask = NULL;
    mOutputTypeSet = false;

    mIsDifferential = false;

    mConvRunType = 1; //'oneway pass';
}

Filtering::~Filtering() {
    return;
    if (mMask != NULL)
        delete[] mMask;
}

float Filtering::createMaskValue(int x, int y, int z){
    return 1;
}

// TODO have to set mRecreateMask to true if input change dimension
void Filtering::createMask(Image::pointer input, uchar maskSize) {
    if (!mRecreateMask)
        return;
    mRuntimeManager->startRegularTimer("create_mask");
    bool separable = isSeparable();
    if (mConvRunType == 0 || mConvRunType == 2){
        mRuntimeManager->startRegularTimer("create_naive_mask");
        unsigned char halfSize = (maskSize - 1) / 2;
        float sum = 0.0f;

        if (input->getDimensions() == 2){
            mMask = new float[maskSize*maskSize];
            for (int x = -halfSize; x <= halfSize; x++){
                for (int y = -halfSize; y <= halfSize; y++){
                    float value = createMaskValue(x, y);
                    //std::cout << "value " << value << std::endl;
                    //float value = 1;
                    //float value = exp(-(float)(x*x + y*y) / (2.0f*mStdDev*mStdDev));
                    mMask[x + halfSize + (y + halfSize)*maskSize] = value; //can also just have a counter for mMask index!

                    sum += value;
                }
            }
            if (!mIsDifferential){
                for (int i = 0; i < maskSize*maskSize; ++i) //starte den her riktig på 0 eller 1?
                    mMask[i] /= sum;
            }
            else {
                for (int i = 0; i < maskSize*maskSize; i++) //starte den her riktig på 0 eller 1?
                    mMask[i] /= 4.0;
            }
        }
        else if (input->getDimensions() == 3){
            mMask = new float[maskSize*maskSize*maskSize];
            for (int x = -halfSize; x <= halfSize; x++){
                for (int y = -halfSize; y <= halfSize; y++){
                    for (int z = -halfSize; z <= halfSize; z++){
                        float value = createMaskValue(x, y, z);
                        //float value = 1;
                        //float value = exp(-(float)(x*x + y*y + z*z) / (2.0f*mStdDev*mStdDev));
                        mMask[x + halfSize + (y + halfSize)*maskSize + (z + halfSize)*maskSize*maskSize] = value; //can also just have a counter
                        sum += value;
                    }
                }
            }
            if (!mIsDifferential){
                for (int i = 0; i < maskSize*maskSize*maskSize; ++i) //starte den her riktig på 0 eller 1?
                    mMask[i] /= sum;
            }

        }
        //mRuntimeManager->stopRegularTimer("create_naive_mask");
        separable = isSeparable();
    }
    else if (mConvRunType == 1 && separable){
        mRuntimeManager->startRegularTimer("create_twopass_mask");
        mMaskX = getSeparable(0);
        mMaskY = getSeparable(1);
        //mRuntimeManager->stopRegularTimer("create_twopass_mask");
    }
    

    

    /*
    //std::cout << "Mask IN FILTERING " << mIsDifferential << mMask << std::endl;
    for (int i = 0; i < maskSize*maskSize; i++){
        float maskVal = mMask[i];
        //std::cout << "Mask " << i << "  " << maskVal << std::endl;
    }*/
    //mRuntimeManager->startRegularTimer("create_clbuffer_mask");
    ExecutionDevice::pointer device = getMainDevice();
    if (!device->isHost()) {
        OpenCLDevice::pointer clDevice = device;
        if (mConvRunType == 0 || mConvRunType == 2){
            unsigned int bufferSize = input->getDimensions() == 2 ? maskSize*maskSize : maskSize*maskSize*maskSize;
            mCLMask = cl::Buffer(
                clDevice->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*bufferSize,
                mMask
                );
            mRuntimeManager->stopRegularTimer("create_naive_mask");
        }
        else if (mConvRunType == 1 && separable){
            unsigned int bufferSizeDir = maskSize;
            mCLMaskX = cl::Buffer(
                clDevice->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*bufferSizeDir,
                mMaskX);
            mCLMaskY = cl::Buffer(
                clDevice->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*bufferSizeDir,
                mMaskY);
            mRuntimeManager->stopRegularTimer("create_twopass_mask");
        }
        
    }
    //mRuntimeManager->stopRegularTimer("create_clbuffer_mask");

    mRecreateMask = false;
    mRuntimeManager->stopRegularTimer("create_mask");
}

bool isSeparableMask(float * filter){ //Or make it Filtering specific?
    return true;
    //True if rank = 1 (or zero?)
}

bool Filtering::isSeparable(){  // OR assign a value when separable
    return isSeparableMask(mMask);
}

float * Filtering::getSeparable(int dir){
    float * mask = new float[mMaskSize];
    // TODO!
    return mask;
}

void Filtering::recompileOpenCLCode(Image::pointer input) {
    // Check if there is a need to recompile OpenCL code
    if (input->getDimensions() == mDimensionCLCodeCompiledFor &&
        input->getDataType() == mTypeCLCodeCompiledFor && mConvRunType == mConvRunTypeCompiledFor)
        return;
    // TODO add more conditions for skipping recompile? Filtersize same etc..
    // AND/OR split up to multiple
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
    if (true){
        int maskSize = int(mMaskSize);
        buildOptions += " -D FILTER_SIZE=";
        buildOptions += std::to_string(maskSize);
        std::cout << "maskSize " << maskSize << std::endl;
        buildOptions += " -D HALF_FILTER_SIZE=";
        buildOptions += std::to_string((maskSize - 1) / 2);
        std::cout << "mIsDifferential " << bool(mIsDifferential) << std::endl;
        if (mIsDifferential){
            buildOptions += " -D STATIC_ADD=0.5";
        }
        else {
            buildOptions += " -D STATIC_ADD=0.0";
        }
        
    }
    
    if (mConvRunType == 1 && isSeparable()){
        std::cout << "Running part " << mConvRunType << std::endl;
        cl::Program programX, programY;
        //assume 2D
        std::string buildOptsX = buildOptions + " -D PASS_DIRECTION=0";
        std::cout << "PasDir build optsX" << buildOptsX << std::endl;
        programX = getOpenCLProgram(device, "2D", buildOptsX);
        std::string buildOptsY = buildOptions + " -D PASS_DIRECTION=1";
        std::cout << "PasDir build optsY" << buildOptsY << std::endl;
        programY = getOpenCLProgram(device, "2D", buildOptsY);


        mKernelDirX = cl::Kernel(programX, "OneDirPass");
        mKernelDirY = cl::Kernel(programY, "OneDirPass");
        //mKernelDirZ = cl::Kernel(program, "OneDirPass");
    }
    else if (mConvRunType == 2){
        // TODO Advnaced method
        //split separable and not?

        std::cout << "Running part " << mConvRunType << std::endl;
        //cl::Program programX, programY;
        cl::Program program;
        
        buildOptions += " -D PASS_DIRECTION=0";
        buildOptions += " -D LOCAL_MEM_PAD=0";
        //assumes 2D for now
        std::cout << "PasDir build optsX" << buildOptions << std::endl;
        program = getOpenCLProgram(device, "2D", buildOptions);
        mKernel = cl::Kernel(program, "FilteringLocalMemory");//todo rename? :P
    }
    else {
        buildOptions += " -D PASS_DIRECTION=0";
        
        cl::Program program;
        if (input->getDimensions() == 2) {
            program = getOpenCLProgram(device, "2D", buildOptions);
        }
        else {
            program = getOpenCLProgram(device, "3D", buildOptions);
        }
        mKernel = cl::Kernel(program, "SpatialFilter"); 
        //mKernel = cl::Kernel(program, "OneDirPass");
    }
    
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
    mConvRunTypeCompiledFor = mConvRunType;
}

template <class T>
void executeAlgorithmOnHost(Image::pointer input, Image::pointer output, float * mask, unsigned char maskSize) {
//template <class T>
//void Filtering::executeAlgorithmOnHost(Image::pointer input, Image::pointer output, float * mask, unsigned char maskSize) {
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

void Filtering::execute() {
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
            fastSwitchTypeMacro(executeAlgorithmOnHost<FAST_TYPE>(input, output, mMask, maskSize));
        }
    }
    else if (mConvRunType == 1 && isSeparable() ){ 
        std::cout << "Running part " << mConvRunType << std::endl;
        mRuntimeManager->startRegularTimer("twopass_setup");
        OpenCLDevice::pointer clDevice = device;

        recompileOpenCLCode(input);
        cl::NDRange globalSize;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        //Assumes 2D
        globalSize = cl::NDRange(input->getWidth(), input->getHeight());
        cl::NDRange localSizeX = cl::NDRange(input->getWidth(), 1);
        cl::NDRange localSizeY = cl::NDRange(1, input->getHeight());
        // Pass 0 - X direction
        Image::pointer tempImage = Image::New();
        /*create(
            unsigned int width,
            unsigned int height,
            DataType type,
            unsigned int nrOfComponents,
            ExecutionDevice::pointer device,
            const void* data) {*/
        /*tempImage->create(
            input->getWidth(),
            input->getHeight(),
            input->getDataType(),
            input->getNrOfComponents(),
            device,
            data?
            );*/
        tempImage->createFromImage(input);
        std::cout << "temp size " << tempImage->getSize() << std::endl;
        OpenCLImageAccess::pointer tempAccess = tempImage->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        mKernelDirX.setArg(0, *inputAccess->get2DImage());
        mKernelDirX.setArg(2, *tempAccess->get2DImage());
        mKernelDirX.setArg(1, mCLMaskX);

        OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        mKernelDirY.setArg(0, *tempAccess->get2DImage());
        mKernelDirY.setArg(2, *outputAccess->get2DImage());
        mKernelDirY.setArg(1, mCLMaskY);

        cl::CommandQueue cmdQueue = clDevice->getCommandQueue();
        cmdQueue.finish();
        //mRuntimeManager->startCLTimer("twopass_cl", cmdQueue);
        mRuntimeManager->startRegularTimer("twopass_cl");
        cmdQueue.enqueueNDRangeKernel(
            mKernelDirX,
            cl::NullRange,
            globalSize,
            localSizeX
            );
        cmdQueue.finish();
        //cmdQueue.enqueueBarrier();
        //clFinish(cmdQueue);

        // Pass 1 - Y direction

        cmdQueue.enqueueNDRangeKernel(
            mKernelDirY,
            cl::NullRange,
            globalSize,
            localSizeY
            );
        cmdQueue.finish();
        //mRuntimeManager->stopCLTimer("twopass_cl", cmdQueue);
        mRuntimeManager->stopRegularTimer("twopass_cl");
        mRuntimeManager->stopRegularTimer("twopass_setup");
    }
    else if (mConvRunType == 2){
        // split separable and not?
        // TODO implement advanced routine 
        std::cout << "Running part " << mConvRunType << std::endl;
        mRuntimeManager->startRegularTimer("local_setup");
        OpenCLDevice::pointer clDevice = device;

        recompileOpenCLCode(input);
        /*
            __kernel void FilteringLocalMemory(
                __read_only image2d_t input,
                __constant float * mask,
                __local float * sharedMem,
                __write_only image2d_t output)
        */
        //assumes 2D
        cl::NDRange globalSize;
        OpenCLImageAccess::pointer inputAccess = input->getOpenCLImageAccess(ACCESS_READ, device);
        OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
        int localWidth = 32;
        int localHeight = 16;
        int globalWidth = input->getWidth() + mMaskSize - 1;
        int globalHeight = input->getHeight() + mMaskSize - 1;
        globalWidth = globalWidth + (localWidth - (globalWidth & (localWidth-1) ) );
        globalHeight = globalHeight + (localHeight - (globalHeight & (localHeight - 1) ) );
        std::cout << "Widths: " << input->getWidth() << " " << localWidth << " " << globalWidth << std::endl;
        std::cout << "Height: " << input->getHeight() << " " << localHeight << " " << globalHeight << std::endl;
        globalSize = cl::NDRange(globalWidth, globalHeight); //buffer to include padding
        cl::NDRange localSize = cl::NDRange(32, 16); // 1D?
        size_t sharedSize = (32 * 16 * sizeof(float));
        mKernel.setArg(0, *inputAccess->get2DImage());
        mKernel.setArg(3, *outputAccess->get2DImage());
        mKernel.setArg(1, mCLMask);
        mKernel.setArg(2, sharedSize, NULL); //sharedMem size initialized
        cl::CommandQueue cmdQueue = clDevice->getCommandQueue();
        //mRuntimeManager->startCLTimer("naive_cl", cmdQueue);
        cmdQueue.finish();
        mRuntimeManager->startRegularTimer("local_cl");
       // cl_int * err;
       // cl::Device::getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(err);
        std::cout << " max work items: " << CL_DEVICE_MAX_WORK_GROUP_SIZE << std::endl;
        cmdQueue.enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            globalSize,
            localSize
            );
        //mRuntimeManager->stopCLTimer("naive_cl", cmdQueue);
        cmdQueue.finish();
        mRuntimeManager->stopRegularTimer("local_cl");
        mRuntimeManager->stopRegularTimer("local_setup");
    }
    else {
        std::cout << "Running part " << mConvRunType << std::endl;
        mRuntimeManager->startRegularTimer("naive_setup");
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
        cl::CommandQueue cmdQueue = clDevice->getCommandQueue();
        //mRuntimeManager->startCLTimer("naive_cl", cmdQueue);
        cmdQueue.finish();
        mRuntimeManager->startRegularTimer("naive_cl");
        cmdQueue.enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            globalSize,
            cl::NullRange
            );
        //mRuntimeManager->stopCLTimer("naive_cl", cmdQueue);
        cmdQueue.finish();
        mRuntimeManager->stopRegularTimer("naive_cl");
        mRuntimeManager->stopRegularTimer("naive_setup");
    }
    std::cout << "maxIntensity! @Filtering " << output->calculateMaximumIntensity() << std::endl;
    //std::cout << "kernel! @Filtering " << output->calculateMaximumIntensity() << std::endl;
}

void Filtering::waitToFinish() {
    if (!getMainDevice()->isHost()) {
        OpenCLDevice::pointer device = getMainDevice();
        device->getCommandQueue().finish();
    }
}
