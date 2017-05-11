#ifndef ELLIPSE_MODEL_HPP
#define ELLIPSE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/ShapeModel.hpp"

namespace fast {

class FAST_EXPORT  EllipseModel : public ShapeModel {
	FAST_OBJECT(EllipseModel)
	public:
		Shape::pointer getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
		MatrixXf getProcessErrorMatrix();
		std::vector<MatrixXf> getMeasurementVectors(VectorXf state, Shape::pointer shape);
		/**
		 * Set initial state in mm
		 */
		void setInitialState(Vector2f position, float majorRadius, float minorRadius);
		VectorXf getInitialState(SharedPointer<Image> image);
		VectorXf restrictState(VectorXf state);
	private:
		EllipseModel();
		Matrix4f mStateTransitionMatrix1;
		Matrix4f mStateTransitionMatrix2;
		Matrix4f mStateTransitionMatrix3;
		Matrix4f mProcessErrorMatrix;
		int mNrOfNodes;

		VectorXf mInitialState;
};

}

#endif
