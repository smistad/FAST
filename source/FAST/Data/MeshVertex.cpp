#include "MeshVertex.hpp"

namespace fast {


MeshVertex::MeshVertex(VectorXf position) {
	mPosition = position;
	mNormal = VectorXf::Zero(mPosition.size());
	mLabel = 0;
	mColor = Color::Red();
}
MeshVertex::MeshVertex(VectorXf position, VectorXf normal) : MeshVertex(position) {
	mNormal = normal;
}

MeshVertex::MeshVertex(VectorXf position, VectorXf normal,
		std::vector<int> connections) : MeshVertex(position, normal) {
	mConnections = connections;
}

void MeshVertex::setColor(Color color) {
	mColor = color;
}

Color MeshVertex::getColor() const {
	return mColor;
}

void MeshVertex::setLabel(int label) {
	mLabel = label;
}

int MeshVertex::getLabel() const {
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
