#ifndef MESH_TO_SEGMENTATION_HPP_
#define MESH_TO_SEGMENTATION_HPP_

#include "FAST/Algorithms/SegmentationAlgorithm.hpp"

namespace fast {

class FAST_EXPORT  MeshToSegmentation : public SegmentationAlgorithm {
	FAST_OBJECT(MeshToSegmentation)
	public:
        /**
         * Set output image resolution in voxels
         * @param x
         * @param y
         * @param z
         */
		void setOutputImageResolution(uint x, uint y, uint z = 1);
	private:
		MeshToSegmentation();
		void execute();

		Vector3i mResolution;

};

}

#endif
