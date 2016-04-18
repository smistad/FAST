#include "MeshAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

MeshAccess::MeshAccess(
        std::vector<MeshVertex>* vertices,
        std::vector<VectorXui>* connections,
        SharedPointer<Mesh> mesh) {
    mVertices = vertices;
    mConnections = connections;
    mMesh = mesh;;
}

void MeshAccess::release() {
	mMesh->accessFinished();
}

MeshAccess::~MeshAccess() {
	release();
}


MeshVertex MeshAccess::getVertex(uint i) {
    return (*mVertices)[i];
}

Vector3ui MeshAccess::getTriangle(uint i) {
    return (*mConnections)[i];
}
Vector2ui MeshAccess::getLine(uint i) {
    return (*mConnections)[i];
}

std::vector<MeshVertex> MeshAccess::getVertices() {
    std::vector<MeshVertex> copy = *mVertices;
    return copy;
}

std::vector<VectorXui> MeshAccess::getTriangles() {
    std::vector<VectorXui> copy = *mConnections;
    return copy;
}
std::vector<VectorXui> MeshAccess::getLines() {
    std::vector<VectorXui> copy = *mConnections;
    return copy;
}

} // end namespace fast


