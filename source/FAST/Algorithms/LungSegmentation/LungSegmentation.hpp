#ifndef FAST_LUNG_SEGMENTATION_HPP_
#define FAST_LUNG_SEGMENTATION_HPP_

#include "FAST/ProcessObject.hpp"

namespace fast {

class LungSegmentation : public ProcessObject {
    FAST_OBJECT(LungSegmentation)
public:
private:
    LungSegmentation();
    void execute();
};

}

#endif