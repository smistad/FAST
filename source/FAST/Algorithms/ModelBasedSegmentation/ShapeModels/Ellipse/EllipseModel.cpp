#include "EllipseModel.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

EllipseModel::EllipseModel() {
	// Create state transition matrices and process error matrix
    float dampening = 0.5; // 0 is no dampening, 1 is full
    mStateTransitionMatrix1 = Matrix4f::Zero();
    mStateTransitionMatrix1(0,0) = 2 - dampening;
    mStateTransitionMatrix1(1,1) = 2 - dampening;
    mStateTransitionMatrix1(2,2) = 0.5;
    mStateTransitionMatrix1(3,3) = 0.5;
    mStateTransitionMatrix2 = Matrix4f::Zero();
    mStateTransitionMatrix2(0,0) = dampening - 1;
    mStateTransitionMatrix2(1,1) = dampening - 1;
    mStateTransitionMatrix2(2,2) = 0.5;
    mStateTransitionMatrix2(3,3) = 0.5;
    mStateTransitionMatrix3 = Matrix4f::Zero();
    mProcessErrorMatrix = Matrix4f::Zero();
    mProcessErrorMatrix(0,0) = 0.01;
    mProcessErrorMatrix(1,1) = 0.01;
    mProcessErrorMatrix(2,2) = 0.01;
    mProcessErrorMatrix(3,3) = 0.01;

    mNrOfNodes = 16;

    mInitialState = Vector4f(0, 0, 1, 1);
}

Shape::pointer EllipseModel::getShape(VectorXf state) {
	// TODO need 2D support in mesh
	Vector2f center(state(0), state(1));
    float flattening = 1.0f - state(3)/state(2);
    float predictedRadius = state(2);

    std::vector<MeshVertex> vertices;
	for(int i = 0; i < mNrOfNodes; ++i) {
        float alpha = 2.0*M_PI*i/mNrOfNodes;
        Vector2f direction(cos(alpha), (1-flattening)*sin(alpha));
        Vector2f position = center + direction*predictedRadius;
        Vector3f position3D(position.x(), position.y(), 0);

        Vector2f normal((1-flattening)*predictedRadius*cos(alpha), predictedRadius*sin(alpha));
        normal.normalize();
        Vector3f normal3D(normal.x(), normal.y(), 0);
        vertices.push_back(MeshVertex(position3D, normal3D));
	}
	std::vector<MeshLine> connections;
	for(int i = 0; i < mNrOfNodes-1; ++i) {
		MeshLine line(i, i+1);
		connections.push_back(line);
	}
	MeshLine line(mNrOfNodes-1, 0);
	connections.push_back(line);

	Mesh::pointer mesh = Mesh::New();
	mesh->create(vertices, connections);
	Shape::pointer shape = Shape::New();
	shape->setMesh(mesh);
	return shape;
}

MatrixXf EllipseModel::getStateTransitionMatrix1() {
	return mStateTransitionMatrix1;
}

MatrixXf EllipseModel::getStateTransitionMatrix2() {
	return mStateTransitionMatrix2;
}

MatrixXf EllipseModel::getStateTransitionMatrix3() {
	return mStateTransitionMatrix3;
}

MatrixXf EllipseModel::getProcessErrorMatrix() {
	return mProcessErrorMatrix;
}

std::vector<MatrixXf> EllipseModel::getMeasurementVectors(VectorXf state,
		Shape::pointer shape) {

    float flattening = 1.0f - state(3)/state(2);
    float predictedRadius = state(2);

	std::vector<MatrixXf> result;
	for(int i = 0; i < mNrOfNodes; ++i) {
        float alpha = 2.0*M_PI*i/mNrOfNodes;
        Vector2f direction(cos(alpha), (1-flattening)*sin(alpha));

        Vector2f normal((1-flattening)*predictedRadius*cos(alpha), predictedRadius*sin(alpha));
        normal.normalize();

        MatrixXf derivativeMatrix = MatrixXf::Zero(2, 4);
        derivativeMatrix.col(0) = Vector2f(1,0);
        derivativeMatrix.col(1) = Vector2f(0,1);
        derivativeMatrix(0,2) = cos(alpha);
        derivativeMatrix(1,3) = sin(alpha);

        MatrixXf measurementVector = normal.transpose()*derivativeMatrix;
        result.push_back(measurementVector);
	}

	return result;
}

VectorXf EllipseModel::getInitialState(Image::pointer image) {
	return mInitialState;
}

void EllipseModel::setInitialState(Vector2f position, float majorRadius, float minorRadius) {
	mInitialState = Vector4f(position.x(), position.y(), majorRadius, minorRadius);
}

VectorXf EllipseModel::restrictState(VectorXf state) {
	if(state(2) < 3.5) {
		state(2) = 3.5;
	}
	if(state(3) < 3.5) {
		state(3) = 3.5;
	}
	if(state(2) > 6) {
		state(2) = 6;
	}
	if(state(3) > 6) {
		state(3) = 6;
	}
	if(state(2) < state(3))
		state(3) = state(2);

	return state;

}

}
