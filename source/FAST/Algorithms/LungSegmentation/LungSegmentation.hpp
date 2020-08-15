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
    DataChannel::pointer getBloodVesselOutputPort();
private:
    LungSegmentation();
    void execute();
    std::shared_ptr<Image> convertToHU(std::shared_ptr<Image> image);

    Vector3i findSeedVoxel(std::shared_ptr<Image> input);

    Vector3i mSeedPoint;
    bool mUseManualSeedPoint = false;
};

}

#endif