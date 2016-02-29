#include "KalmanFilter.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Data/Mesh.hpp"
#include "Shape.hpp"

namespace fast {

void KalmanFilter::predict() {
	// Use temporal/motion model to predict the next state and covariance
	// This is done using matrices from the shape model
	MatrixXf A1 = mShapeModel->getStateTransitionMatrix1();
	MatrixXf A2 = mShapeModel->getStateTransitionMatrix2();
	MatrixXf A3 = mShapeModel->getStateTransitionMatrix3();
	mPredictedState = A1*mCurrentState + A2*mPreviousState + A3*mDefaultState;
	mPredictedCovariance = A1*mCurrentCovariance*A1.transpose() + A2*mPreviousCovariance*A2.transpose() +
			A1*mCurrentCovariance*A2.transpose() + A2*mPreviousCovariance*A1.transpose() + mShapeModel->getProcessErrorMatrix();
}

void KalmanFilter::setShapeModel(ShapeModel::pointer shapeModel) {
	mShapeModel = shapeModel;
}

void KalmanFilter::setAppearanceModel(
		AppearanceModel::pointer appearanceModel) {
	mAppearanceModel = appearanceModel;
}

void KalmanFilter::execute() {
    reportInfo() << "Executing Kalman filter.." << reportEnd();
	if(!mShapeModel.isValid() || !mAppearanceModel.isValid())
		throw Exception("Shape and appearance model must be given to the Kalman filter before execution.");

	Image::pointer image = getStaticInputData<Image>();

	int counter = 5;
	while(counter--) {
		predict();
		estimate(image);
	}
	reportInfo() << "Current state3: " << mCurrentState.transpose() << reportEnd();
    reportInfo() << "Finished one round of Kalman filter" << reportEnd();

	Shape::pointer shape = mShapeModel->getShape(mCurrentState);
	setStaticOutputData<Mesh>(0, shape->getMesh());
}

KalmanFilter::KalmanFilter() {
	createInputPort<Image>(0);
	createOutputPort<Mesh>(0, OUTPUT_DEPENDS_ON_INPUT, 0);
}

void KalmanFilter::estimate(SharedPointer<Image> image) {

	Shape::pointer shape = mShapeModel->getShape(mPredictedState);
	std::vector<Measurement> measurements = mAppearanceModel->getMeasurements(image, shape);
	std::vector<MatrixXf> measurementVectors = mShapeModel->getMeasurementVectors(mPredictedState, shape);

	// Assimilate the measurements into HRH and HRv
	const uint nrOfMeasurements = measurements.size();
	const uint stateSize = mPredictedState.size();
	MatrixXf HRH = MatrixXf::Zero(stateSize, stateSize);
	VectorXf HRv = VectorXf::Zero(stateSize);
	for(uint i = 0; i < nrOfMeasurements; ++i) {
		MatrixXf measurementVector = measurementVectors[i];
		if(measurements[i].uncertainty < 1) {
			// TODO what is the 0.001 factor here??
			HRH += measurementVector.transpose()*(0.001*1.0f/measurements[i].uncertainty)*measurementVector;
			HRv += measurementVector.transpose()*(0.001*1.0f/measurements[i].uncertainty)*measurements[i].displacement;
		}
	}

	// Update covariance and state
	mPreviousState = mCurrentState;
	mPreviousCovariance = mCurrentCovariance;
	mCurrentCovariance = (mPredictedCovariance.inverse() + HRH).inverse();
	mCurrentState = mPredictedState + mCurrentCovariance*HRv;

}

void KalmanFilter::placeShapeInImageCenter() {
    reportInfo() << "In place shape in image center" << reportEnd();
    try {
		getInputPort(0).getProcessObject()->update();// Make sure we have a frame;
    } catch(...) {
    	throw Exception("Must call setInputConnection before placeShapeInImageCenter");
    }
    reportInfo() << "In place shape in image center" << reportEnd();
    Image::pointer currentFrame = DynamicData::pointer(getInputPort(0).getData())->getCurrentFrame();
    Vector3f volumeCentroid;
    volumeCentroid[0] = currentFrame->getWidth() / 2;
    volumeCentroid[1] = currentFrame->getHeight() / 2;
    volumeCentroid[2] = currentFrame->getDepth() / 2;
    VectorXf defaultState = mShapeModel->getState(Vector3f::Zero(), Vector3f(0.8, 0.8, 0.8), Vector3f::Zero());
    Shape::pointer defaultShape = mShapeModel->getShape(defaultState);
    Vector3f modelCentroid = defaultShape->getCentroid();

    AffineTransformation::pointer transformMatrix = SceneGraph::getAffineTransformationFromData(currentFrame);
    transformMatrix->scale(currentFrame->getSpacing());
    volumeCentroid = transformMatrix->multiply(volumeCentroid);
    Vector3f translation = volumeCentroid - modelCentroid;

    // TODO should probably initialize states and covariance when loading shape model
    mCurrentState = mShapeModel->getState(translation, Vector3f(0.8, 0.8, 0.8), Vector3f::Zero());
    mPreviousState = mCurrentState;
    mDefaultState = mCurrentState;
    mCurrentCovariance = MatrixXf::Zero(mCurrentState.size(), mCurrentState.size());
    mPreviousCovariance = mCurrentCovariance;
    reportInfo() << "Finished placing shape in image center" << reportEnd();
}

}
