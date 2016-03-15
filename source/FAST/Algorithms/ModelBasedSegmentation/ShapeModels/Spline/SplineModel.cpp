#include "SplineModel.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

inline Vector2f transformPosition(Vector2f position, Matrix2f RS, Vector2f T, Vector2f C) {
	return RS*(position - C) + C + T;
}

Shape::pointer SplineModel::getShape(VectorXf state) {

	// Create mesh
	std::vector<Vector2f> vertices;
	std::vector<Vector2f> normals;
	std::vector<VectorXui> lines;

	int nrOfControlPoints = mControlPoints.size();

	Matrix2f scaling = Matrix2f::Zero();
	scaling(0, 0) = state(2);
	scaling(1, 1) = state(3);
	Matrix2f rotation = Eigen::Rotation2Df(state(4)).toRotationMatrix();

	Matrix2f RS = rotation*scaling;

	Vector2f T(state(0), state(1));

	int counter = 0;
	for(int c = 0; c < nrOfControlPoints; ++c) {
		for(int i = 1; i < mResolution+1; ++i) {
			// Neighbor control points
			Vector2f y0, y1, y2, y3;
			int statePosition;
			if(c - 1 < 0) {
				statePosition = 5 + (nrOfControlPoints - 1)*2;
				y0 = mControlPoints[nrOfControlPoints - 1] + Vector2f(state(statePosition), state(statePosition+1));
			} else {
				statePosition = 5 + (c - 1)*2;
				y0 = mControlPoints[c - 1] + Vector2f(state(statePosition), state(statePosition+1));
			}
			statePosition = 5 + c*2;
	        y1 = mControlPoints[c] + Vector2f(state(statePosition), state(statePosition+1));
			statePosition = 5 + ((c+1) % (nrOfControlPoints))*2;
	        y2 = mControlPoints[(c+1) % (nrOfControlPoints)] + Vector2f(state(statePosition), state(statePosition+1));
			statePosition = 5 + ((c+2) % (nrOfControlPoints))*2;
	        y3 = mControlPoints[(c+2) % (nrOfControlPoints)] + Vector2f(state(statePosition), state(statePosition+1));

			// Apply global deformation to control points
			y0 = transformPosition(y0, RS, T, mCentroid);
			y1 = transformPosition(y1, RS, T, mCentroid);
			y2 = transformPosition(y2, RS, T, mCentroid);
			y3 = transformPosition(y3, RS, T, mCentroid);

	        // Basis functions
	        float u = (float)i/mResolution;
	        float a0 =  2*u*u*u - 3*u*u + 1;
	        float a1 =    u*u*u - 2*u*u + u;
	        float a2 =    u*u*u -   u*u;
	        float a3 = -2*u*u*u + 3*u*u;

	        // This is just the derivate of that above
	        float at0 =  6*u*u - 6*u;
	        float at1 = 3*u*u - 4*u + 1;
	        float at2 =    3*u*u -   2*u;
	        float at3 = -6*u*u + 6*u;

			// Control point tangents calculated using central difference
			Vector2f m0 = ((1-mTension)/2)*(y2-y0);
			Vector2f m1 = ((1-mTension)/2)*(y3-y1);

			// Calculate point p on spline
			Vector2f vertex =  a0*y1 + a1*m0 + a2*m1 + a3*y2;
			// Calculate tangent for point p on spline
			Vector2f tangent = at0*y1 + at1*m0 + at2*m1 + at3*y2;
			tangent.normalize();
			Vector2f normal(-tangent.y(), tangent.x());

			vertices.push_back(vertex);
			normals.push_back(normal);
			// TODO check this:
			if(counter < nrOfControlPoints*mResolution-1) {
				lines.push_back(Vector2ui(counter, counter + 1));
			} else {
				lines.push_back(Vector2ui(counter, 0));
			}
			counter++;
		}
	}

	Mesh::pointer mesh = Mesh::New();
	mesh->create(vertices, normals, lines);

	Shape::pointer shape = Shape::New();
	shape->setMesh(mesh);
	return shape;
}

MatrixXf SplineModel::getStateTransitionMatrix1() {
	return mA1;
}

MatrixXf SplineModel::getStateTransitionMatrix2() {
	return mA2;
}

MatrixXf SplineModel::getStateTransitionMatrix3() {
	return mA3;
}

MatrixXf SplineModel::getProcessErrorMatrix() {
	return mProcessErrorMatrix;
}

VectorXf SplineModel::getInitialState(SharedPointer<Image> image) {
	VectorXf defaultState = VectorXf::Zero(mStateSize);
	// Set scale to 1
	defaultState(2) = 0.02;
	defaultState(3) = 0.03;
	// Rotate 180 degrees
	defaultState(4) = M_PI;

	if(mInitializeShapeToImageCenter) {
		Vector3f spacing = image->getSpacing();
		Vector2f imageCentroid;
		imageCentroid[0] = spacing.x() * image->getWidth() / 2;
		imageCentroid[1] = spacing.y() * image->getHeight() / 2;

		//Shape::pointer defaultShape = getShape(defaultState);
		//Vector3f modelCentroid = defaultShape->getCentroid();

		//Vector2f translation = imageCentroid - modelCentroid.head(2);
		Vector2f translation = imageCentroid - mCentroid;
		defaultState(0) = translation(0);
		defaultState(1) = translation(1);
	}
	return defaultState;
}

std::vector<Vector2f> SplineModel::getLocallyDeformedVertices(VectorXf state) {
	std::vector<Vector2f> vertices;
	int nrOfControlPoints = mControlPoints.size();
	mCentroid = Vector2f::Zero();
	for(int c = 0; c < nrOfControlPoints; ++c) {
		for(int i = 1; i < mResolution+1; ++i) {
			// Neighbor control points
			Vector2f y0, y1, y2, y3;
			int statePosition;
			if(c - 1 < 0) {
				statePosition = 5 + (nrOfControlPoints - 1)*2;
				y0 = mControlPoints[nrOfControlPoints - 1] + Vector2f(state(statePosition), state(statePosition+1));
			} else {
				statePosition = 5 + (c - 1)*2;
				y0 = mControlPoints[c - 1] + Vector2f(state(statePosition), state(statePosition+1));
			}
			statePosition = 5 + c*2;
	        y1 = mControlPoints[c] + Vector2f(state(statePosition), state(statePosition+1));
			statePosition = 5 + ((c+1) % (nrOfControlPoints))*2;
	        y2 = mControlPoints[(c+1) % (nrOfControlPoints)] + Vector2f(state(statePosition), state(statePosition+1));
			statePosition = 5 + ((c+2) % (nrOfControlPoints))*2;
	        y3 = mControlPoints[(c+2) % (nrOfControlPoints)] + Vector2f(state(statePosition), state(statePosition+1));

			// Basis functions
	        float u = (float)i/mResolution;
	        float a0 =  2*u*u*u - 3*u*u + 1;
	        float a1 =    u*u*u - 2*u*u + u;
	        float a2 =    u*u*u -   u*u;
	        float a3 = -2*u*u*u + 3*u*u;

			// Control point tangents calculated using central difference
			Vector2f m0 = ((1-mTension)/2)*(y2-y0);
			Vector2f m1 = ((1-mTension)/2)*(y3-y1);

			// Calculate point p on spline
			Vector2f vertex =  a0*y1 + a1*m0 + a2*m1 + a3*y2;
			vertices.push_back(vertex);
			mCentroid += vertex;
		}
	}

	mCentroid /= vertices.size();
	return vertices;
}

std::vector<MatrixXf> SplineModel::getMeasurementVectors(VectorXf state,
		Shape::pointer shape) {

	const float sx = state(2);
	const float sy = state(3);
	const double r = state(4);
	Matrix2f scaling = Matrix2f::Zero();
	scaling(0, 0) = sx;
	scaling(1, 1) = sy;
	Matrix2f rotation = Eigen::Rotation2Df(r).toRotationMatrix();

	Matrix2f RS = rotation*scaling;

	std::vector<MatrixXf> result;

	int nrOfControlPoints = mControlPoints.size();

	MeshAccess::pointer access = shape->getMesh()->getMeshAccess(ACCESS_READ);

	std::vector<Vector2f> locallyDeformedVertices = getLocallyDeformedVertices(state);

	int counter = 0;
	for(int c = 0; c < nrOfControlPoints; ++c) {
		for(int i = 1; i < mResolution+1; ++i) {
			Vector2f normal = access->getVertex(counter).getNormal();
			VectorXf h = VectorXf::Zero(mStateSize);

			// GLOBAL PART
			MatrixXf globalPart = MatrixXf::Zero(2, 5);
			globalPart(0, 0) = 1;
			globalPart(1, 1) = 1;

			Vector2f pMinusC = locallyDeformedVertices[counter] - mCentroid;
			// The partial derivatives of RS
			Matrix2f dRS_sx, dRS_sy, dRS_r;
			dRS_sx << cos(r), 0,
					  sin(r), 0;
			dRS_sy << 0, -sin(r),
					  0, cos(r);
			dRS_r << -sin(r)*sx, -cos(r)*sy,
					  cos(r)*sx, -sin(r)*sy;
			Vector2f scalingXPart = dRS_sx * pMinusC;
			Vector2f scalingYPart = dRS_sy * pMinusC;
			Vector2f rotationPart = dRS_r * pMinusC;
			globalPart(0, 2) = scalingXPart.x();
			globalPart(1, 2) = scalingXPart.y();
			globalPart(0, 3) = scalingYPart.x();
			globalPart(1, 3) = scalingYPart.y();
			globalPart(0, 4) = rotationPart.x();
			globalPart(1, 4) = rotationPart.y();

			h.head(5) = normal.transpose()*globalPart;

			// LOCAL PART

			// Neighbor control points indices
			int y0, y1, y2, y3;
			if(c - 1 < 0) {
				y0 = nrOfControlPoints - 1;
			} else {
				y0 = c - 1;
			}
	        y1 = c;
	        y2 = (c+1) % (nrOfControlPoints);
	        y3 = (c+2) % (nrOfControlPoints);

	        // Basis functions
	        float u = (float)i/mResolution;
	        float a0 =  2*u*u*u - 3*u*u + 1;
	        float a1 =    u*u*u - 2*u*u + u;
	        float a2 =    u*u*u -   u*u;
	        float a3 = -2*u*u*u + 3*u*u;

	        // Put in tension factor
	        a1 *= (1.0f-mTension)/2.0f;
	        a2 *= (1.0f-mTension)/2.0f;


			MatrixXf localJacobian = MatrixXf::Zero(2, nrOfControlPoints*2);
			localJacobian(0, y0*2) = -a1;
			localJacobian(1, y0*2+1) = -a1;
			localJacobian(0, y1*2) = a0-a2;
			localJacobian(1, y1*2+1) = a0-a2;
			localJacobian(0, y2*2) = a1+a3;
			localJacobian(1, y2*2+1) = a1+a3;
			localJacobian(0, y3*2) = a2;
			localJacobian(1, y3*2+1) = a2;
			VectorXf localPart = normal.transpose()*RS*localJacobian;
			h.tail(h.size()-5) = localPart;

			//std::cout << "Measurement vector: " << h.transpose() << std::endl;

			result.push_back(MatrixXf(h.transpose()));
			counter++;
		}
	}

	return result;
}

void SplineModel::initializeShapeToImageCenter() {
	mInitializeShapeToImageCenter = true;
}

SplineModel::SplineModel() {
	mInitializeShapeToImageCenter = false;

	// Create control points
	std::vector<Vector2f> controlPoints = {
	        Vector2f(0, 3), // Apex
	        Vector2f(0.8, 2),
	        Vector2f(1.15, 1),
	        Vector2f(0.8, 0),
	        Vector2f(-0.8, 0),
	        Vector2f(-1.15, 1),
	        Vector2f(-0.8, 2)
	};

	// Calculate centroid
	mCentroid = Vector2f::Zero();
	for(Vector2f p : controlPoints) {
		mCentroid += p;
	}
	mCentroid /= controlPoints.size();

	mControlPoints = controlPoints;
	mStateSize = 5+2*controlPoints.size();
	mResolution = 8;
	mTension = 0;

	    // Create state transition matrices
    const float dampening = 0.5;

    mA1 = MatrixXf::Zero(mStateSize,mStateSize);
    mA1(0,0) = 1+dampening; // translation x
    mA1(1,1) = 1+dampening; // translation y
    mA1(2,2) = 1+dampening; // scaling x
    mA1(3,3) = 1+dampening; // scaling y
    mA1(4,4) = 0.5; // rotation
    for(int i = 5; i < mStateSize; ++i)
        mA1(i,i) = 0.5;

    mA2 = MatrixXf::Zero(mStateSize,mStateSize);
    mA2(0,0) = -dampening;
    mA2(1,1) = -dampening;
    mA2(2,2) = -dampening;
    mA2(3,3) = -dampening;
    mA2(4,4) = 0.5;//-dampening;
    for(int i = 5; i < mStateSize; ++i) {
        mA2(i,i) = 0.5;
    }

    mA3 = MatrixXf::Zero(mStateSize, mStateSize);

    mProcessErrorMatrix = MatrixXf::Zero(mStateSize, mStateSize);
    for(int i = 0; i < mStateSize; ++i) {
    	if(i < 5) {
    		mProcessErrorMatrix(i, i) = 0.01f;
    	} else {
    		mProcessErrorMatrix(i, i) = 0.001f;
    	}
    }
}


}
