#ifndef STEP_EDGE_MODEL_HPP
#define STEP_EDGE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/AppearanceModel.hpp"

namespace fast {

class Image;
class Shape;

class StepEdgeModel : public AppearanceModel {
	FAST_OBJECT(StepEdgeModel)
	public:
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		std::vector<Measurement> getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape);
	private:
		StepEdgeModel();

		float mLineLength;
		float mLineSampleSpacing;
		float mIntensityDifferenceThreshold;

};

}

#endif
