#ifndef KALMAN_FILTER_HPP
#define KALMAN_FILTER_HPP

#include "FAST/ProcessObject.hpp"
#include "AppearanceModel.hpp"
#include "ShapeModel.hpp"

namespace fast {

class Image;

class KalmanFilter : public ProcessObject {
	FAST_OBJECT(KalmanFilter)
	public:
		void setShapeModel(ShapeModel::pointer shapeModel);
		void setAppearanceModel(AppearanceModel::pointer appearanceModel);
	private:
		KalmanFilter();
		void execute(); // runs a loop with predict, measure and update
		void predict();
		void estimate(SharedPointer<Image> image);

		AppearanceModel::pointer mAppearanceModel;
		ShapeModel::pointer mShapeModel;

		VectorXf mCurrentState;
		VectorXf mPreviousState;
		VectorXf mDefaultState;
		VectorXf mPredictedState;

		MatrixXf mCurrentCovariance;
		MatrixXf mPreviousCovariance;
		MatrixXf mPredictedCovariance;

		bool mInitialized;
};

} // end namespace fast

#endif
