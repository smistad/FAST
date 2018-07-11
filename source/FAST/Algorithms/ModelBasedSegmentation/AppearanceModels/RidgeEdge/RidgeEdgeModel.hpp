#ifndef RIDGE_EDGE_MODEL_HPP
#define RIDGE_EDGE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/AppearanceModel.hpp"

namespace fast {

class Image;
class Shape;

class FAST_EXPORT  RidgeEdgeModel : public AppearanceModel {
	FAST_OBJECT(RidgeEdgeModel)
	public:
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
        void setMinimumRidgeSize(float sizeInMM);
		std::vector<Measurement> getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape, ExecutionDevice::pointer device);
        enum EdgeType {
			EDGE_TYPE_ANY,
			EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE,
			EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE
		};
		void setEdgeType(EdgeType type);
	private:
		RidgeEdgeModel();
        std::vector<Measurement> getMeasurementsOnHost(SharedPointer<Image> image, SharedPointer<Shape> shape);
        std::vector<Measurement> getMeasurementsOnDevice(SharedPointer<Image> image, SharedPointer<Shape> shape, OpenCLDevice::pointer device);
        int convertRidgeSizeToSamples();

		float mLineLength;
		float mLineSampleSpacing;
		float mIntensityDifferenceThreshold;
		float mMinimumDepth;
    	float mRidgeSize;
        EdgeType mEdgeType;
};

}

#endif
