#ifndef SHAPE_MODEL_HPP_
#define SHAPE_MODEL_HPP_

#include "FAST/Data/DataTypes.hpp"
#include "Shape.hpp"

namespace fast {

class Image;

/**
 * This is an abstract base class for all shape models.
 * These objects describe a deformable shape, its parameters
 * and how it is allowed to change in space and time.
 */
class FAST_EXPORT  ShapeModel : public Object {
	public:
		typedef SharedPointer<ShapeModel> pointer;
		virtual Shape::pointer getShape(VectorXf state) = 0;
		virtual MatrixXf getStateTransitionMatrix1() = 0;
		virtual MatrixXf getStateTransitionMatrix2() = 0;
		virtual MatrixXf getStateTransitionMatrix3() = 0;
		virtual MatrixXf getProcessErrorMatrix() = 0;
		virtual VectorXf getInitialState(SharedPointer<Image> image) = 0;
		virtual std::vector<MatrixXf> getMeasurementVectors(VectorXf state, Shape::pointer shape) = 0;
		virtual VectorXf restrictState(VectorXf state) { return state; };
	private:

};

}

#endif
