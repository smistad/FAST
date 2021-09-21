#pragma once

#include "FAST/Algorithms/KalmanFilterModelSegmentation/ShapeModel.hpp"

namespace fast {

/**
 * @brief Cardinal spline shape model
 *
 * Used in the Kalman filter deformable model segmentation algorithm
 *
 * @sa KalmanFilter
 */
class FAST_EXPORT  CardinalSplineModel : public ShapeModel {
	FAST_OBJECT_V4(CardinalSplineModel)
	public:
        /**
         * @brief Create instance
         * @param controlPoints List of control points for the cardinal spline.
         * @param globalProcessError
         * @param localProcessError
         * @return instance
         */
        FAST_CONSTRUCTOR(CardinalSplineModel,
             std::vector<Vector2f>, controlPoints,,
             float, globalProcessError, = 0.000001f,
             float, localProcessError, = 0.0000001f
        );
		Shape::pointer getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
		MatrixXf getProcessErrorMatrix();
		VectorXf getInitialState(std::shared_ptr<Image> image);
		std::vector<MatrixXf> getMeasurementVectors(VectorXf state, Shape::pointer shape);
		void initializeShapeToImageCenter();
		/**
		 * Give a set of control points.
		 * The control points are assumed to be connected so that vector k,
		 * is connected to vector k-1 and k+1, and the last is connected to
		 * to the first to form a closed loop.
		 */
		void setControlPoints(std::vector<Vector2f> controlPoints);
		void setInitialScaling(float x, float y);
		void setInitialRotation(float angleInRadians);
		void setInitialTranslation(float x, float y);
		void setLocalProcessError(float error);
		void setGlobalProcessError(float error);
		/*
		 * Set a single tension for all control points
		 */
		void setTension(float tension);
		/*
		 * Set the tension value for each control point. This vector has to be
		 * the same size as the number of control points.
		 */
		void setTension(std::vector<float> tension);
		void setResolution(int resolution);
		VectorXf restrictState(VectorXf state);
		/*
		 * Set min and max limit for scaling
		 */
		void setScalingLimit(float min = -1, float max = -1);
	private:
		void assertControlPointsGiven();
		void updateProcessErrorMatrix();
		std::vector<Vector2f> getLocallyDeformedVertices(VectorXf state);
		std::vector<float> getTensionVector(uint nrOfControlPoints) const;

		MatrixXf mA1;
		MatrixXf mA2;
		MatrixXf mA3;
		MatrixXf mProcessErrorMatrix;
		float mLocalProcessError;
		float mGlobalProcessError;
		VectorXf mInitialGlobalState;
		int mStateSize;
		int mResolution;
		float mMinScaling, mMaxScaling;

		std::vector<Vector2f> mControlPoints;
		float mGlobalTension;
		std::vector<float> mTensionPerVertex;
		Vector2f mCentroid;
		bool mClockwise; // True if control points are in clockwise order

		bool mInitializeShapeToImageCenter;

};

}
