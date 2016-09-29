#ifndef MESH_TO_SEGMENTATION_HPP_
#define MESH_TO_SEGMENTATION_HPP_

#include "FAST/Algorithms/SegmentationAlgorithm.hpp"

namespace fast {

class MeshToSegmentation : public SegmentationAlgorithm {
	FAST_OBJECT(MeshToSegmentation)
	public:
	private:
		MeshToSegmentation();
		void execute();

};

}

#endif
