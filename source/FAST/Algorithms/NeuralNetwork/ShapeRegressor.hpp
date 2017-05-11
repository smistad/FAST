#ifndef SHAPE_REGRESSOR_HPP_
#define SHAPE_REGRESSOR_HPP_

#include "FAST/ProcessObject.hpp"
#include "FAST/Algorithms/NeuralNetwork/NeuralNetwork.hpp"

namespace fast {

class FAST_EXPORT  ShapeRegressor : public NeuralNetwork {
	FAST_OBJECT(ShapeRegressor)
	public:
	private:
		ShapeRegressor();
		void execute();
};

}

#endif
