#ifndef STEP_EDGE_MODEL_HPP
#define STEP_EDGE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/AppearanceModel.hpp"

namespace fast {

class StepEdgeModel : public AppearanceModel {
	public:
		void getMeasurements(SharedPointer<Image> image, SharedPointer<MeshShape> shape);
	private:

};

}

#endif
