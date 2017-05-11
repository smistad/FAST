#ifndef FAST_LUNG_SEGMENTATION_HPP_
#define FAST_LUNG_SEGMENTATION_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

class FAST_EXPORT  LungSegmentation : public ProcessObject {
    FAST_OBJECT(LungSegmentation)
public:
private:
    LungSegmentation();
    void execute();

    Vector3i findSeedVoxel(SharedPointer<Image> input);
};

}

#endif