#include "MeshVertex.hpp"

namespace fast {


MeshVertex::MeshVertex(Vector3f position) {
	mPosition = position;
	mNormal = Vector3f(1, 0, 0);
	mLabel = 0;
	mColor = Color::Red();
}
MeshVertex::MeshVertex(Vector3f position, Vector3f normal) : MeshVertex(position) {
	mNormal = normal;
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

Vector3f MeshVertex::getPosition() const {
	return mPosition;
}

Vector3f MeshVertex::getNormal() const {
	return mNormal;
}

void MeshVertex::setNormal(Vector3f normal) {
	mNormal = normal;
}

void MeshVertex::setPosition(Vector3f position) {
	mPosition = position;
}

int MeshConnection::getEndpoint(uint index) {
    return mEndpoints[index];
}

int MeshConnection::getEndpoint1() {
    return mEndpoints[0];
}

int MeshConnection::getEndpoint2() {
    return mEndpoints[1];
}

Color MeshConnection::getColor() {
    return mColor;
}

void MeshConnection::setEndpoint(int endpointIndex, int vertexIndex) {
    mEndpoints[endpointIndex] = vertexIndex;
}

void MeshConnection::setEndpoint1(uint index) {
    mEndpoints[0] = index;

}

void MeshConnection::setEndpoint2(uint index) {
    mEndpoints[1] = index;
}

void MeshConnection::setColor(Color color) {
    mColor = color;
}

MeshLine::MeshLine(uint endpoint1, uint endpoint2, Color color) {
    mEndpoints = VectorXui::Zero(2);
    mEndpoints[0] = endpoint1;
    mEndpoints[1] = endpoint2;
    setColor(color);
}

MeshTriangle::MeshTriangle(uint endpoint1, uint endpoint2, uint endpoint3, Color color) {
    mEndpoints = VectorXui::Zero(3);
    mEndpoints[0] = endpoint1;
    mEndpoints[1] = endpoint2;
    mEndpoints[2] = endpoint3;
    setColor(color);
}

int MeshTriangle::getEndpoint3() {
    return mEndpoints[2];
}

void MeshTriangle::setEndpoint3(uint index) {
    mEndpoints[2] = index;
}

}
