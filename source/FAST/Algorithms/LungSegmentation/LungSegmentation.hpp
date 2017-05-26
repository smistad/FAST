#ifndef FAST_LUNG_SEGMENTATION_HPP_
#define FAST_LUNG_SEGMENTATION_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

class FAST_EXPORT LungSegmentation : public ProcessObject {
    FAST_OBJECT(LungSegmentation)
public:
    void setAirwaySeedPoint(int x, int y, int z);
    void setAirwaySeedPoint(Vector3i seed);
private:
    LungSegmentation();
    void execute();
    SharedPointer<Image> convertToHU(SharedPointer<Image> image);

    Vector3i findSeedVoxel(SharedPointer<Image> input);

    Vector3i mSeedPoint;
    bool mUseManualSeedPoint = false;
};

}

#endif