#ifndef CARDINAL_SPLINE_MODEL_HPP
#define CARDINAL_SPLINE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/ShapeModel.hpp"

namespace fast {

class FAST_EXPORT  CardinalSplineModel : public ShapeModel {
	FAST_OBJECT(CardinalSplineModel)
	public:
		Shape::pointer getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
		MatrixXf getProcessErrorMatrix();
		VectorXf getInitialState(SharedPointer<Image> image);
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
		CardinalSplineModel();
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

#endif
