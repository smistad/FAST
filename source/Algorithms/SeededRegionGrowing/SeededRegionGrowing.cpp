#include "SeededRegionGrowing.hpp"
#include "DynamicImage.hpp"
#include "DeviceManager.hpp"

namespace fast {


void SeededRegionGrowing::setInput(ImageData::pointer input) {
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

void SeededRegionGrowing::setIntensityRange(float min, float max) {
    if(min >= max)
        throw Exception("Min must be smaller than max intensity range in SeededRegionGrowing");

    mMinimumIntensity = min;
    mMaximumIntensity = max;
    mIsModified = true;
}

void SeededRegionGrowing::addSeedPoint(uint x, uint y) {
    Uint<3> pos;
    pos[0] = x;
    pos[1] = y;
    pos[2] = 0;
    addSeedPoint(pos);
}

void SeededRegionGrowing::addSeedPoint(uint x, uint y, uint z) {
    Uint<3> pos;
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    addSeedPoint(pos);
}

void SeededRegionGrowing::addSeedPoint(Uint<3> position) {
    mSeedPoints.push_back(position);
}

void SeededRegionGrowing::setDevice(ExecutionDevice::pointer device) {
    mDevice = device;
    mIsModified = true;
}

ImageData::pointer SeededRegionGrowing::getOutput() {
    if(!mOutput.isValid()) {
        throw Exception("Must call setInput before getOutput in SeededRegionGrowing");
    }
    return mOutput;
}

SeededRegionGrowing::SeededRegionGrowing() {
    mDevice = DeviceManager::getInstance().getDefaultComputationDevice();
    mDimensionCLCodeCompiledFor = 0;
}

void SeededRegionGrowing::recompileOpenCLCode(Image::pointer input) {
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
        filename = "Algorithms/SeededRegionGrowing/SeededRegionGrowing2D.cl";
    } else {
        filename = "Algorithms/SeededRegionGrowing/SeededRegionGrowing3D.cl";
    }
    int programNr = device->createProgramFromSource(std::string(FAST_SOURCE_DIR) + filename, buildOptions);
    mKernel = cl::Kernel(device->getProgram(programNr), "seededRegionGrowing");
    mDimensionCLCodeCompiledFor = input->getDimensions();
    mTypeCLCodeCompiledFor = input->getDataType();
}

void SeededRegionGrowing::execute() {
    Image::pointer input;
    if(!mInput.isValid())
        throw Exception("No input supplied to SeededRegionGrowing");

    if(mSeedPoints.size() == 0)
        throw Exception("No seed points supplied to SeededRegionGrowing");

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
    if(input->getDimensions() == 2) {
        output->create2DImage(
                input->getWidth(),
                input->getHeight(),
                TYPE_UINT8,
                1,
                mDevice
        );
    } else {
         output->create3DImage(
                input->getWidth(),
                input->getHeight(),
                input->getDepth(),
                TYPE_UINT8,
                1,
                mDevice
        );
    }

    if(mDevice->isHost()) {
        throw Exception("SeededRegionGrowing not implemented for host yet");
    } else {
        OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);

        recompileOpenCLCode(input);

        // add seeds
        ImageAccess access = output->getImageAccess(ACCESS_READ_WRITE);
        uchar* outputData = (uchar*)access.get();
        for(int i = 0; i < mSeedPoints.size(); i++) {
            Uint<3> pos = mSeedPoints[i];
            outputData[pos.x() + pos.y()*output->getWidth() + pos.z()*output->getWidth()*output->getHeight()] = 2;
        }
        access.release();

        OpenCLImageAccess2D inputAccess = input->getOpenCLImageAccess2D(ACCESS_READ, device);
        OpenCLBufferAccess outputAccess = output->getOpenCLBufferAccess(ACCESS_READ_WRITE, device);

        cl::Buffer stopGrowingBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_WRITE,
                sizeof(char));
        mKernel.setArg(0, *inputAccess.get());
        mKernel.setArg(1, *outputAccess.get());
        mKernel.setArg(2, stopGrowingBuffer);
        mKernel.setArg(3, mMinimumIntensity);
        mKernel.setArg(4, mMaximumIntensity);

        cl::CommandQueue queue = device->getCommandQueue();
        bool stopGrowing = false;
        char stopGrowingInit = 1;
        char * stopGrowingResult = new char;
        int iterations = 0;
        do {
            iterations++;
            queue.enqueueWriteBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), &stopGrowingInit);

            std::cout << "iteration" << std::endl;
            cl::NDRange globalSize;
            globalSize = cl::NDRange(input->getWidth(),input->getHeight());
            queue.enqueueNDRangeKernel(
                    mKernel,
                    cl::NullRange,
                    globalSize,
                    cl::NullRange
            );

            queue.enqueueReadBuffer(stopGrowingBuffer, CL_TRUE, 0, sizeof(char), stopGrowingResult);
            if(*stopGrowingResult == 1)
                stopGrowing = true;
        } while(!stopGrowing);
        std::cout << "total iterations: " << iterations << std::endl;
    }

    if(!mInput->isDynamicData())
        mInput->release(mDevice);

    // Update the timestamp of the output data
    output->updateModifiedTimestamp();
}

void SeededRegionGrowing::waitToFinish() {
    if(!mDevice->isHost()) {
        OpenCLDevice::pointer device = boost::static_pointer_cast<OpenCLDevice>(mDevice);
        device->getCommandQueue().finish();
    }
}

} // end namespace fast
