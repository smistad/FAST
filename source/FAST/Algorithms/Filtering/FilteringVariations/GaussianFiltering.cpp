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
}

GaussianFiltering::~GaussianFiltering() {
    delete[] mMask;
}

float GaussianFiltering::createMaskValue(int x, int y, int z){
    if (z==0)
        return exp(-(float)(x*x + y*y) / (2.0f*mStdDev*mStdDev));
    else
        return exp(-(float)(x*x + y*y + z*z) / (2.0f*mStdDev*mStdDev)); //*mStdDev?
}

bool GaussianFiltering::isSeparable(){
    return true;
}

float * GaussianFiltering::getSeparable(int dir){
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
    for (int i = 0; i < maskSize; i++){
        mask[i] /= sum;
    }
    return mask;
}