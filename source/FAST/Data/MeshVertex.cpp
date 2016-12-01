#include "MeshVertex.hpp"

namespace fast {


MeshVertex::MeshVertex(VectorXf position) {
	mPosition = position;
	mNormal = VectorXf::Zero(mPosition.size());
	mLabel = 0;
}
MeshVertex::MeshVertex(VectorXf position, VectorXf normal) {
	mPosition = position;
	mNormal = normal;
	mLabel = 0;
}

MeshVertex::MeshVertex(VectorXf position, VectorXf normal,
		std::vector<int> connections) {
	mPosition = position;
	mNormal = normal;
	mConnections = connections;
	mLabel = 0;
}

void MeshVertex::setLabel(int label) {
	mLabel = label;
}

int MeshVertex::getLabel() {
	return mLabel;
}

uchar MeshVertex::getNrOfDimensions() const {
	return mPosition.size();
}

VectorXf MeshVertex::getPosition() const {
	return mPosition;
}

VectorXf MeshVertex::getNormal() const {
	return mNormal;
}

void MeshVertex::setNormal(VectorXf normal) {
	mNormal = normal;
}

void MeshVertex::setPosition(VectorXf position) {
	mPosition = position;
}

std::vector<int> MeshVertex::getConnections() const {
	return mConnections;
}

void MeshVertex::addConnection(int index) {
	mConnections.push_back(index);
}

}
