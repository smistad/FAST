#include "FAST/Algorithms/Filtering/FilteringVariations/DifferentialFiltering.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

/*
void DifferentialFiltering::setDirection(int direction) {
    if (direction < 0)
        throw Exception("Standard deviation of DifferentialFiltering can't be less than 0.");

    mDirection = direction;
    mIsModified = true;
    mRecreateMask = true;
}*/


DifferentialFiltering::DifferentialFiltering() : Filtering() {
    mDifferentialAddValue = 128;
    //mDirection = 0; // left to right (instead of top to down)
    //mMaskSize = 3; //default for sobel
    //mStdDev = 0.5f;
    //mMaskDefaultSize = (char)(ceil(2 * mStdDev) * 2 + 1);
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

// TODO have to set mRecreateMask to true if input change dimension
void DifferentialFiltering::createMask(Image::pointer input, uchar maskSize) {
    if (!mRecreateMask)
        return;

    unsigned char halfSize = (maskSize - 1) / 2;
    //float sum = 0.0f;

    if (input->getDimensions() == 2){
        mMask = new float[maskSize*maskSize];
        for (int x = -halfSize; x <= halfSize; x++){
            for (int y = -halfSize; y <= halfSize; y++){
                float value = createMaskValue(x, y);
                std::cout << "value " << value << std::endl;
                //float value = 1;
                //float value = exp(-(float)(x*x + y*y) / (2.0f*mStdDev*mStdDev));
                mMask[x + halfSize + (y + halfSize)*maskSize] = value; //can also just have a counter for mMask index!

                //sum += value;
            }
        }
        //for (int i = 0; i < maskSize*maskSize; ++i) //starte den her riktig på 0 eller 1?
            //mMask[i] /= sum;
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
                    //sum += value;
                }
            }
        }
        //for (int i = 0; i < maskSize*maskSize*maskSize; ++i) //starte den her riktig på 0 eller 1?
            //mMask[i] /= sum;
    }

    std::cout << "Mask " << mMask << std::endl;
    for (int i = 0; i < maskSize*maskSize; i++){
        int maskVal = int(mMask[i]);
        std::cout << "Mask " << i << "  " << maskVal << std::endl;
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

DifferentialFiltering::~DifferentialFiltering() {
    delete[] mMask;
}

