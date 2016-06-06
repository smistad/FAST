#ifndef SEGMENTATION_ALGORITHM_HPP
#define SEGMENTATION_ALGORITHM_HPP

#include "FAST/ProcessObject.hpp"
#include "FAST/Data/Segmentation.hpp"

namespace fast {

class SegmentationAlgorithm : public ProcessObject {
    public:
        virtual void setLabel(Segmentation::LabelType label);
        virtual Segmentation::LabelType getLabel() const;
    protected:
        Segmentation::LabelType mLabel;
        SegmentationAlgorithm() : mLabel(Segmentation::LABEL_FOREGROUND) {};
};

}

#endif
