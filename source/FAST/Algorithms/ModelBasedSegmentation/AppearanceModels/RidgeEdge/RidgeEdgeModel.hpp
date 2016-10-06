#ifndef RIDGE_EDGE_MODEL_HPP
#define RIDGE_EDGE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/AppearanceModel.hpp"

namespace fast {

class Image;
class Shape;

class RidgeEdgeModel : public AppearanceModel {
	FAST_OBJECT(RidgeEdgeModel)
	public:
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
    	void setRidgeSizes(std::vector<float> sizes);
		std::vector<Measurement> getMeasurements(SharedPointer<Image> image, SharedPointer<Shape> shape, ExecutionDevice::pointer device);
	private:
		RidgeEdgeModel();
        std::vector<Measurement> getMeasurementsOnHost(SharedPointer<Image> image, SharedPointer<Shape> shape);
        std::vector<Measurement> getMeasurementsOnDevice(SharedPointer<Image> image, SharedPointer<Shape> shape, OpenCLDevice::pointer device);

		float mLineLength;
		float mLineSampleSpacing;
		float mIntensityDifferenceThreshold;
		float mMinimumDepth;
    	std::vector<float> mSizes;

};

}

#endif
