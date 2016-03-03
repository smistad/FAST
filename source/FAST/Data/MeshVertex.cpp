#include "MeshVertex.hpp"

namespace fast {


MeshVertex::MeshVertex(VectorXf position) {
	mPosition = position;
	mNormal = VectorXf::Zero(mPosition.size());
}
MeshVertex::MeshVertex(VectorXf position, VectorXf normal) {
	mPosition = position;
	mNormal = normal;
}

MeshVertex::MeshVertex(VectorXf position, VectorXf normal,
		std::vector<int> connections) {
	MeshVertex(position, normal);
	mConnections = connections;
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
