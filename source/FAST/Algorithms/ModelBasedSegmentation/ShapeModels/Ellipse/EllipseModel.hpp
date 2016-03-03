#ifndef ELLIPSE_MODEL_HPP
#define ELLIPSE_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/ShapeModel.hpp"

namespace fast {

class EllipseModel : public ShapeModel {
	FAST_OBJECT(EllipseModel)
	public:
		Shape::pointer getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
		MatrixXf getProcessErrorMatrix();
		std::vector<MatrixXf> getMeasurementVectors(VectorXf state, Shape::pointer shape);
		VectorXf getState(Vector3f translation, Vector3f scale, Vector3f rotation);
	private:
		EllipseModel();
		Matrix4f mStateTransitionMatrix1;
		Matrix4f mStateTransitionMatrix2;
		Matrix4f mStateTransitionMatrix3;
		Matrix4f mProcessErrorMatrix;
		int mNrOfNodes;
};

}

#endif
