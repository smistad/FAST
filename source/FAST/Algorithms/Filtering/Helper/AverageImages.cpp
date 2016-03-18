#include "FAST/Algorithms/Filtering/Helper/AverageImages.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"

using namespace fast;

void AverageImages::setCutOverhead(bool cut){
    cutOverhead = cut;
}

AverageImages::AverageImages() {
    createInputPort<Image>(0); 
    createInputPort<Image>(1);
    createOutputPort<Image>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
    createOpenCLProgram(std::string(FAST_SOURCE_DIR) + "Algorithms/Filtering/Helper/AvgKernel.cl", "2D");
    //3D version?
    
    mDimensionCLCodeCompiledFor = 0;
    mOutputTypeSet = false;
}

AverageImages::~AverageImages() {
    
}

void AverageImages::execute(){
    Image::pointer input = getStaticInputData<Image>(0);
    Image::pointer input1 = getStaticInputData<Image>(1);
    // use multiple if averaging
    Image::pointer output = getStaticOutputData<Image>(0);

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
    SceneGraph::setParentNode(output, input1);

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
        OpenCLImageAccess::pointer input1Access = input1->getOpenCLImageAccess(ACCESS_READ, device);
        float maxVal = input->calculateMaximumIntensity();
        float maxVal1 = input1->calculateMaximumIntensity();
        float minVal = input->calculateMinimumIntensity();
        float minVal1 = input1->calculateMinimumIntensity();
        if (input->getDimensions() == 2) {
            globalSize = cl::NDRange(input->getWidth(), input->getHeight());

            OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
            mKernel.setArg(0, *inputAccess->get2DImage());
            mKernel.setArg(1, *input1Access->get2DImage());
            mKernel.setArg(2, *outputAccess->get2DImage());
            mKernel.setArg(3, maxVal);
            mKernel.setArg(4, maxVal1);
            mKernel.setArg(5, maxVal1);
            mKernel.setArg(6, maxVal1);
        }
        else {
            globalSize = cl::NDRange(input->getWidth(), input->getHeight(), input->getDepth());

            mKernel.setArg(0, *inputAccess->get3DImage());
            mKernel.setArg(1, *input1Access->get3DImage());
            if (clDevice->isWritingTo3DTexturesSupported()) {
                OpenCLImageAccess::pointer outputAccess = output->getOpenCLImageAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess->get3DImage());
            }
            else {
                OpenCLBufferAccess::pointer outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);
                mKernel.setArg(2, *outputAccess->get());
            }
        }

        clDevice->getCommandQueue().enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            globalSize,
            cl::NullRange
            );
    }
    std::cout << "maxIntensity! @avgAdd " << output->calculateMaximumIntensity() << std::endl;
    std::cout << "minIntensity! @avgAdd " << output->calculateMinimumIntensity() << std::endl;
}

void AverageImages::recompileOpenCLCode(Image::pointer input) {
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
    if (true){
        buildOptions += " -D MAX_VAL=1.0";
        buildOptions += " -D MIN_VAL=0.0";
        if (cutOverhead){
            buildOptions += " -D CUT_OVERHEAD=true";
        }
        else {
            buildOptions += " -D CUT_OVERHEAD=false";
        }
    }
    cl::Program program;
    if (input->getDimensions() == 2) {
        program = getOpenCLProgram(device, "2D", buildOptions);
    }
    else {
        program = getOpenCLProgram(device, "3D", buildOptions);
    }
    mKernel = cl::Kernel(program, "CutExcess");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
}
