#pragma once

#include <FAST/Algorithms/SegmentationAlgorithm.hpp>

namespace fast {

class TissueSegmentation : public SegmentationAlgorithm {
    FAST_OBJECT(TissueSegmentation)
    public:
    protected:
        void execute();
        TissueSegmentation();

};

}