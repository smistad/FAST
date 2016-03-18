#include "FAST/Algorithms/Filtering/FilteringVariations/SobelFiltering.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Data/Image.hpp"
using namespace fast;

void SobelFiltering::setDirection(int direction) {
if (direction < 0)
throw Exception("Standard deviation of SobelFiltering can't be less than 0.");
if (direction == mDirection) return;
mDirection = direction;
mIsModified = true;
mRecreateMask = true;
mNeedRebuild = true;
}


SobelFiltering::SobelFiltering() : Filtering() {
    mDirection = 0; // left to right (instead of top to down)
    mMaskSize = 3; //default for sobel
    mIsDifferential = true;
}

SobelFiltering::~SobelFiltering() {
    delete[] mMask;
}

//returns whole number
float SobelFiltering::createMaskValue(int x, int y, int z){
    if (mDirection == 0){ // along X axis (left / right)
        return (-x) * (2 - abs(y)) * (2 - abs(z));
    }
    else if (mDirection == 1){ //along Y axis (top / down)
        return (2 - abs(x)) * (-y) * (2 - abs(z));
    }
    else if (mDirection == 2){ // along Z axis (back / forth)
        return (2 - abs(x)) * (2 - abs(y)) * (-z);
    }
    return 0; // can be stupid just leave one of the else ifs as "else"
}
/*
// TODO have to set mRecreateMask to true if input change dimension
void SobelFiltering::createMask(Image::pointer input, uchar maskSize) {
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

bool SobelFiltering::isSeparable(){
    return true;
}

float * SobelFiltering::getSeparable(int dir){
    float * mask = new float[3];
    if (dir == mDirection){ // Differential direction
        mask[0] = 1.0;
        mask[1] = 0.0;
        mask[2] = -1.0;
        return mask;
    }
    else { // Gaussian smoothing direction
        mask[0] = 0.25;
        mask[1] = 0.5;
        mask[2] = 0.25;
        return mask;
    }
}
