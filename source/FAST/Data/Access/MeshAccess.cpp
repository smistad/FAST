#include "MeshAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

MeshAccess::MeshAccess(
        std::vector<MeshVertex>* vertices,
        std::vector<Vector3ui>* triangles,
        SharedPointer<Mesh> mesh) {
    mVertices = vertices;
    mTriangles = triangles;
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
    return (*mTriangles)[i];
}

std::vector<MeshVertex> MeshAccess::getVertices() {
    std::vector<MeshVertex> copy = *mVertices;
    return copy;
}

std::vector<Vector3ui> MeshAccess::getTriangles() {
    std::vector<Vector3ui> copy = *mTriangles;
    return copy;
}

} // end namespace fast


