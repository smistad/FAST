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
	private:
		CardinalSplineModel();
		std::vector<Vector2f> getLocallyDeformedVertices(VectorXf state);

		MatrixXf mA1;
		MatrixXf mA2;
		MatrixXf mA3;
		MatrixXf mProcessErrorMatrix;
		int mStateSize;
		int mResolution;

		std::vector<Vector2f> mControlPoints;
		float mTension;
		Vector2f mCentroid;

		bool mInitializeShapeToImageCenter;

};

}

#endif
