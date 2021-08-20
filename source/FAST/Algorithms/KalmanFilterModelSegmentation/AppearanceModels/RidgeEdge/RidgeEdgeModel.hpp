#pragma once

#include "FAST/Algorithms/KalmanFilterModelSegmentation/AppearanceModel.hpp"

namespace fast {

class Image;
class Shape;


/**
 * @brief Ridge edge appearance model
 *
 * Used in Kalman filter deformable model segmentation
 *
 * @sa KalmanFilter StepEdgeModel
 */
class FAST_EXPORT RidgeEdgeModel : public AppearanceModel {
	FAST_OBJECT_V4(RidgeEdgeModel)
	public:
        enum EdgeType {
            EDGE_TYPE_ANY,
            EDGE_TYPE_BLACK_INSIDE_WHITE_OUTSIDE,
            EDGE_TYPE_WHITE_INSIDE_BLACK_OUTSIDE
        };
        FAST_CONSTRUCTOR(RidgeEdgeModel,
                         float, lineLength,,
                         float, lineSampleSpacing,,
                         float, intensityDifferenceThreshold, = 20,
                         float, minimumRidgeSize, = 1,
                         EdgeType, edgeType, = EDGE_TYPE_ANY
        )
		void setLineLength(float lengthInMM);
		void setLineSampleSpacing(float spacingInMM);
		void setIntensityDifferenceThreshold(float threshold);
		void setMinimumDepth(float depth);
        void setMinimumRidgeSize(float sizeInMM);
		std::vector<Measurement> getMeasurements(std::shared_ptr<Image> image, std::shared_ptr<Shape> shape, ExecutionDevice::pointer device);

		void setEdgeType(EdgeType type);
	private:
        std::vector<Measurement> getMeasurementsOnHost(std::shared_ptr<Image> image, std::shared_ptr<Shape> shape);
        std::vector<Measurement> getMeasurementsOnDevice(std::shared_ptr<Image> image, std::shared_ptr<Shape> shape, OpenCLDevice::pointer device);
        int convertRidgeSizeToSamples();

		float mLineLength;
		float mLineSampleSpacing;
		float mIntensityDifferenceThreshold;
		float mMinimumDepth;
    	float mRidgeSize;
        EdgeType mEdgeType;
};

}
