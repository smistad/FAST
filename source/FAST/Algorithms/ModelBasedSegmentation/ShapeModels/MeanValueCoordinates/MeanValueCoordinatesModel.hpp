#ifndef MEAN_VALUE_COORDINATES_MODEL_HPP
#define MEAN_VALUE_COORDINATES_MODEL_HPP

#include "FAST/Algorithms/ModelBasedSegmentation/ShapeModel.hpp"

namespace fast {

class MeanValueCoordinatesModel : ShapeModel {
	public:
		typedef SharedPointer<MeanValueCoordinatesModel> pointer;
		Shape getShape(VectorXf state);
		MatrixXf getStateTransitionMatrix1();
		MatrixXf getStateTransitionMatrix2();
		MatrixXf getStateTransitionMatrix3();
	private:

};

}

#endif
