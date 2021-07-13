#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Extract the skeleton of 2D segmentation as a 2D image
 *
 * Inputs:
 * - 0: Image segmentation 2D
 *
 * Outputs:
 * - 0: Image Skeleton of input 0
 *
 * @ingroup segmentation
 */
class FAST_EXPORT  Skeletonization : public ProcessObject {
    FAST_PROCESS_OBJECT(Skeletonization)
    public:
        FAST_CONSTRUCTOR(Skeletonization)
    private:
        void execute();
};

} // end namespace fast
