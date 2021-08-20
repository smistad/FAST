#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Perform segmentation by following inverse direction of gradient vector field
 *
 * Used in TubeSegmentationAndCenterlineExtraction
 *
 */
class FAST_EXPORT  InverseGradientSegmentation : public ProcessObject {
    FAST_PROCESS_OBJECT(InverseGradientSegmentation)
    public:
        FAST_CONSTRUCTOR(InverseGradientSegmentation)
        void setCenterlineInputConnection(DataChannel::pointer port);
        void setVectorFieldInputConnection(DataChannel::pointer port);
    private:
        void execute();

};

}
