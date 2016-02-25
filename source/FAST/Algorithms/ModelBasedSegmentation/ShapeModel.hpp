#ifndef SHAPE_MODEL_HPP_
#define SHAPE_MODEL_HPP_

#include "FAST/Data/DataTypes.hpp"

namespace fast {

/**
 * This is an abstract base class for all shape models.
 * These objects describe a deformable shape, its parameters
 * and how it is allowed to change in space and time.
 */
class ShapeModel {
	public:
		typedef SharedPointer<ShapeModel> pointer;
		virtual Shape::pointer getShape(VectorXf state) = 0;
		virtual MatrixXf getStateTransitionMatrix1() = 0;
		virtual MatrixXf getStateTransitionMatrix2() = 0;
		virtual MatrixXf getStateTransitionMatrix3() = 0;
		virtual std::vector<VectorXf> getMeasurementVectors(VectorXf state, Shape::pointer shape) = 0;
	private:

};

}

#endif
