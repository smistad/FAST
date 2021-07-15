#pragma once

#include "FAST/Algorithms/KalmanFilterModelSegmentation/AppearanceModel.hpp"

namespace fast {

class Image;
class Shape;

/**
 * @brief Step edge appearance model
 *
 * Used in Kalman filter deformable model segmentation
 *
 * @sa KalmanFilter
 */
class FAST_EXPORT  StepEdgeModel : public AppearanceModel {
	FAST_OBJECT_V4(StepEdgeModel)
	public:
        enum EdgeType {
            EDGE_TYPE_ANY,
            EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE,
            EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE
        };
        FAST_CONSTRUCTOR(StepEdgeModel,
            float, lineLength,,
            float, lineSampleSpacing,,
            float, intensityDifferenceThreshold, = 20,
            EdgeType, edgeType, = EDGE_TYPE_ANY
        )
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
		std::vector<Measurement> getMeasurements(std::shared_ptr<Image> image, std::shared_ptr<Shape> shape, ExecutionDevice::pointer device);
		void setEdgeType(EdgeType type);
	private:

		float mLineLength;
		float mLineSampleSpacing;
		float mIntensityDifferenceThreshold;
		float mMinimumDepth;
		EdgeType mEdgeType;

};

}
