#ifndef STEP_EDGE_MODEL_HPP
#define STEP_EDGE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/AppearanceModel.hpp"

namespace fast {

class Image;
class MeshShape;

class StepEdgeModel : public AppearanceModel {
	FAST_OBJECT(StepEdgeModel)
	public:
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		std::vector<Measurement> getMeasurements(SharedPointer<Image> image, SharedPointer<MeshShape> shape);
	private:
		StepEdgeModel();

		float mLineLength;
		float mLineSampleSpacing;

};

}

#endif
