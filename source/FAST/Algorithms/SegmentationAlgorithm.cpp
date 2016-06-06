#include "FAST/Algorithms/SegmentationAlgorithm.hpp"

namespace fast {

void SegmentationAlgorithm::setLabel(Segmentation::LabelType label) {
    mLabel = label;
}

Segmentation::LabelType SegmentationAlgorithm::getLabel() const {
    return mLabel;
}

}

