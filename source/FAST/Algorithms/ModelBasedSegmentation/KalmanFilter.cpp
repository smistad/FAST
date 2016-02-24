#include "KalmanFilter.hpp"

namespace fast {

void KalmanFilter::predictState() {
	// Use temporal/motion model to predict the next state and covariance
	// This is done using matrices from the shape model
	mPredictedState = mShapeModel->getStateTransitionMatrix1()*mCurrentState +
			mShapeModel->getStateTransitionMatrix2()*mPreviousState +
			mShapeModel->getStateTransitionMatrix3()*mDefaultState;
	mPredictedCovariance;
}

AssimilatedMeasurements KalmanFilter::getMeasurements(Image::pointer image) {

	// Use predicted state and appearance model to get measurements
	return mAppearanceModel->getMeasurements(image, mShapeModel->getShape(mPredictedState));
}

void KalmanFilter::updateState() {
	mPreviousState = mCurrentState;
	// Should shape model be used to create new states?
	// Should state in the Kalman filter just be vectors?
	mCurrentState = ;

}

}
