#ifndef STEP_EDGE_MODEL_HPP
#define STEP_EDGE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/AppearanceModel.hpp"

namespace fast {

class Image;
class Shape;

class FAST_EXPORT  StepEdgeModel : public AppearanceModel {
	FAST_OBJECT(StepEdgeModel)
	public:
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
		std::vector<Measurement> getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape, ExecutionDevice::pointer device);
		enum EdgeType {
			EDGE_TYPE_ANY,
			EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE,
			EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE
		};
		void setEdgeType(EdgeType type);
	private:
		StepEdgeModel();

		float mLineLength;
		float mLineSampleSpacing;
		float mIntensityDifferenceThreshold;
		float mMinimumDepth;
		EdgeType mEdgeType;

};

}

#endif
