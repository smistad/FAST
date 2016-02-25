#include "KalmanFilter.hpp"

namespace fast {

void KalmanFilter::predictState() {
	// Use temporal/motion model to predict the next state and covariance
	// This is done using matrices from the shape model
	MatrixXf A1 = mShapeModel->getStateTransitionMatrix1();
	MatrixXf A2 = mShapeModel->getStateTransitionMatrix2();
	MatrixXf A3 = mShapeModel->getStateTransitionMatrix3();
	mPredictedState = A1*mCurrentState + A2*mPreviousState + A3*mDefaultState;
	mPredictedCovariance = A1*mCurrentState*A1.transpose() + A2*mPredictedCovariance*A2.transpose() +
			A1*mCurrentState*A2.transpose() + A2*mPredictedCovariance*A1.transpose() + mProcessErrorMatrix;
}

void KalmanFilter::updateState(SharedPointer<Image> image) {

	Shape::pointer shape = mShapeModel->getShape(mPredictedState);
	std::vector<Measurement> measurements = mAppearanceModel->getMeasurements(image, shape);
	std::vector<VectorXf> measurementVectors = mShapeModel->getMeasurementVectors(mPredictedState, shape);

	// Assimilate the measurements into HRH and HRv
	const uint nrOfMeasurements = measurements.size();
	const uint stateSize = mPredictedState.size();
	MatrixXf HRH = MatrixXf::Zero(stateSize, stateSize);
	VectorXf HRv = VectorXf::Zero(stateSize);
	for(uint i = 0; i < nrOfMeasurements; ++i) {
		VectorXf measurementVector = measurementVectors[i];
		HRH += measurementVector.transpose()*(1.0f/measurements[i].uncertainty)*measurementVector;
		HRv += measurementVector.transpose()*(1.0f/measurements[i].uncertainty)*measurements[i].displacement;
	}

	// Update covariance and state
	mPreviousState = mCurrentState;
	mCurrentCovariance = (mPredictedCovariance.inverse() + HRH).inverse();
	mCurrentState = mPredictedState + mCurrentCovariance*HRv;

}

}
