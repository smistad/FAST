#pragma once

#include "FAST/ProcessObject.hpp"

namespace fast {

/**
 * @brief Converts a line (2D) or triangle (3D) mesh to a segmentation image
 *
 * To set the output size of segmentation image, either supply an image to input port 1, or manually specify the size.
 *
 * Inputs:
 * - 0: Mesh to convert
 * - 1: (Optional) Image to get size of output from
 *
 * Outputs:
 * - 0: Image - Segmentation image
 *
 * @ingroup segmentation
 */
class FAST_EXPORT MeshToSegmentation : public ProcessObject {
	FAST_PROCESS_OBJECT(MeshToSegmentation)
	public:
        FAST_CONSTRUCTOR(MeshToSegmentation, Vector3i, size, = Vector3i::Zero())
        /**
         * Set output image resolution in voxels
         * @param x
         * @param y
         * @param z
         */
		void setOutputImageResolution(uint x, uint y, uint z = 1);
	private:
		void execute();

		Vector3i mResolution;

};

}
