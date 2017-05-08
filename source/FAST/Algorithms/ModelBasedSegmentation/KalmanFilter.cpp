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

	mPredictedState = mShapeModel->restrictState(mPredictedState);
}

void KalmanFilter::setShapeModel(ShapeModel::pointer shapeModel) {
	mShapeModel = shapeModel;
}

void KalmanFilter::setAppearanceModel(
		AppearanceModel::pointer appearanceModel) {
	mAppearanceModel = appearanceModel;
}

VectorXf KalmanFilter::getCurrentState() const {
	if(!mInitialized)
		throw Exception("Can't get current state before first execute in Kalman filter.");
	return mCurrentState;
}

void KalmanFilter::execute() {
    reportInfo() << "Executing Kalman filter.." << reportEnd();
	if(!mShapeModel.isValid() || !mAppearanceModel.isValid())
		throw Exception("Shape and appearance model must be given to the Kalman filter before execution.");

	Image::pointer image = getStaticInputData<Image>();

	if(!mInitialized) {
		// Initialize state using shape model
		mCurrentState = mShapeModel->getInitialState(image);
		mPreviousState = mCurrentState;
		mDefaultState = mCurrentState;
		mCurrentCovariance = MatrixXf::Zero(mCurrentState.size(), mCurrentState.size());
		mPreviousCovariance = mCurrentCovariance;
		mInitialized = true;
	}

	int counter;
	if(mFirstExecute) {
		counter = mIterations;
		mFirstExecute = false;
	} else {
		counter = mStartIterations;
	}
	while(counter--) {
		predict();
		estimate(image);
	}
	reportInfo() << "Current state: " << mCurrentState.transpose() << reportEnd();
    reportInfo() << "Finished one round of Kalman filter" << reportEnd();
    if(mOutputDisplacements) {
    	Mesh::pointer displacements = getDisplacementVectors(image);
		setStaticOutputData<Mesh>(1, displacements);
    }

	Shape::pointer shape = mShapeModel->getShape(mCurrentState);
	setStaticOutputData<Mesh>(0, shape->getMesh());
}

Mesh::pointer KalmanFilter::getDisplacementVectors(Image::pointer image) {

	Shape::pointer shape = mShapeModel->getShape(mCurrentState);
	Mesh::pointer mesh = shape->getMesh();
	MeshAccess::pointer access = mesh->getMeshAccess(ACCESS_READ);
	std::vector<Measurement> measurements = mAppearanceModel->getMeasurements(image, shape, getMainDevice());

	std::vector<MeshVertex> vertices;
	std::vector<MeshLine> lines;
	const uint nrOfMeasurements = measurements.size();
	if(nrOfMeasurements != mesh->getNrOfVertices())
		throw Exception("Number of vertex inconsistency");
	int counter = 0;
	for(uint i = 0; i < nrOfMeasurements; ++i) {
		Measurement m = measurements[i];
		if(m.uncertainty < 1) {
			MeshVertex vertex = access->getVertex(i);
			MeshVertex v0(vertex.getPosition());
			MeshVertex v1(vertex.getPosition() + m.displacement*vertex.getNormal());
			vertices.push_back(v0);
			vertices.push_back(v1);
			lines.push_back(MeshLine(counter, counter+1));
			counter += 2;
		}
	}

	Mesh::pointer output = Mesh::New();
	output->create(vertices, lines);
	return output;
}

KalmanFilter::KalmanFilter() {
	createInputPort<Image>(0);
	createOutputPort<Mesh>(0, OUTPUT_DEPENDS_ON_INPUT, 0); // Segmentation
	createOutputPort<Mesh>(1, OUTPUT_DEPENDS_ON_INPUT, 0); // Displacement

	mInitialized = false;
	mFirstExecute = true;
	mOutputDisplacements = false;
	mStartIterations = 20;
	mIterations = 5;
}

ProcessObjectPort KalmanFilter::getSegmentationOutputPort() {
	return getOutputPort(0);
}

ProcessObjectPort KalmanFilter::getDisplacementsOutputPort() {
	mOutputDisplacements = true;
	return getOutputPort(1);
}

void KalmanFilter::setIterations(int iterations) {
	if(iterations <= 0)
		throw Exception("Kalman filter iterations must be > 0");

	mIterations = iterations;
}

void KalmanFilter::setStartIterations(int iterations) {
	if(iterations <= 0)
		throw Exception("Kalman filter iterations must be > 0");

	mStartIterations = iterations;
}

void KalmanFilter::estimate(SharedPointer<Image> image) {

	Shape::pointer shape = mShapeModel->getShape(mPredictedState);
	std::vector<Measurement> measurements = mAppearanceModel->getMeasurements(image, shape, getMainDevice());
	std::vector<MatrixXf> measurementVectors = mShapeModel->getMeasurementVectors(mPredictedState, shape);

	// Assimilate the measurements into HRH and HRv
	const uint nrOfMeasurements = measurements.size();
	const uint stateSize = mPredictedState.size();
	MatrixXf HRH = MatrixXf::Zero(stateSize, stateSize);
	VectorXf HRv = VectorXf::Zero(stateSize);
	for(uint i = 0; i < nrOfMeasurements; ++i) {
		MatrixXf measurementVector = measurementVectors[i];
		if(measurements[i].uncertainty < 1) {
			HRH += measurementVector.transpose()*(1.0f/measurements[i].uncertainty)*measurementVector;
			HRv += measurementVector.transpose()*(1.0f/measurements[i].uncertainty)*measurements[i].displacement;
		}
	}


	// Update covariance and state
	mPreviousState = mCurrentState;
	mPreviousCovariance = mCurrentCovariance;
	mCurrentCovariance = (mPredictedCovariance.inverse() + HRH).inverse();
	mCurrentState = mPredictedState + mCurrentCovariance*HRv;
	mCurrentState = mShapeModel->restrictState(mCurrentState);
}

}
