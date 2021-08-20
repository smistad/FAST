#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {
/**
 * @brief Extract 3D centerline by following ridge
 *
 * Used in TubeSegmentationAndCenterlineExtraction
 *
 */
class FAST_EXPORT  RidgeTraversalCenterlineExtraction : public ProcessObject {
    FAST_PROCESS_OBJECT(RidgeTraversalCenterlineExtraction)
    public:
        FAST_CONSTRUCTOR(RidgeTraversalCenterlineExtraction)
    private:
        void execute();
};

}
