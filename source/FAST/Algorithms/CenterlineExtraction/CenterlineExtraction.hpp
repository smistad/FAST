#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

class Image;

/**
 * @brief Extract centerline from 3D image segmentation
 *
 * Uses fast marching algorithm for centerline extraction.
 * Based on the algorithm described in the article
 * "FAST 3D CENTERLINE COMPUTATION FOR TUBULAR STRUCTURES BY FRONT COLLAPSING AND FAST MARCHING" by Cárdenes et al. 2010.
 *
 * Inputs:
 * - 0: Image 3D segmentation
 *
 * Outputs:
 * - 0: Mesh centerlines
 *
 * @ingroup segmentation
 */
class FAST_EXPORT CenterlineExtraction : public ProcessObject {
	FAST_PROCESS_OBJECT(CenterlineExtraction)
    public:
        /**
         * @brief Create instance
         * @return instance
         */
        FAST_CONSTRUCTOR(CenterlineExtraction);
    private:
		void execute();
        std::shared_ptr<Image> calculateDistanceTransform(std::shared_ptr<Image> input);
};

}
