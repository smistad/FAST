#ifndef KALMAN_FILTER_HPP
#define KALMAN_FILTER_HPP

#include "FAST/ProcessObject.hpp"
#include "AppearanceModel.hpp"
#include "ShapeModel.hpp"

namespace fast {

class KalmanFilter : public ProcessObject {
	FAST_OBJECT(KalmanFilter)
	public:
	private:
		void execute(); // runs a loop with predict, measure and update
		void predictState();
		AssimilatedMeasurements getMeasurements();
		void updateState(AssimilatedMeasurements);

		AppearanceModel::pointer mAppearanceModel;
		ShapeModel::pointer mShapeModel;

		VectorXf mCurrentState;
		VectorXf mPreviousState;
		VectorXf mDefaultState;
		VectorXf mPredictedState;

		MatrixXf mCurrentCovariance;
		MatrixXf mPreviousCovariance;
		MatrixXf mPredictedCovariance;
};

}

#endif
