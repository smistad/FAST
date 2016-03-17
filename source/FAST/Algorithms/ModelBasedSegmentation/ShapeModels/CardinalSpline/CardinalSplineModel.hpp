#ifndef CARDINAL_SPLINE_MODEL_HPP
#define CARDINAL_SPLINE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/ShapeModel.hpp"

namespace fast {

class CardinalSplineModel : public ShapeModel {
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
		void setTension(float tension);
		void setResolution(int resolution);
	private:
		CardinalSplineModel();
		void assertControlPointsGiven();
		void updateProcessErrorMatrix();
		std::vector<Vector2f> getLocallyDeformedVertices(VectorXf state);

		MatrixXf mA1;
		MatrixXf mA2;
		MatrixXf mA3;
		MatrixXf mProcessErrorMatrix;
		float mLocalProcessError;
		float mGlobalProcessError;
		VectorXf mInitialGlobalState;
		int mStateSize;
		int mResolution;

		std::vector<Vector2f> mControlPoints;
		float mTension;
		Vector2f mCentroid;

		bool mInitializeShapeToImageCenter;

};

}

#endif
