#include "SurfacePointerAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

SurfacePointerAccess::SurfacePointerAccess(
        std::vector<MeshVertex>* vertices,
        std::vector<Vector3ui>* triangles,
        SharedPointer<Mesh> mesh) {
    mVertices = vertices;
    mTriangles = triangles;
    mMesh = mesh;;
}

void SurfacePointerAccess::release() {
	mMesh->accessFinished();
}

SurfacePointerAccess::~SurfacePointerAccess() {
	release();
}


MeshVertex SurfacePointerAccess::getVertex(uint i) {
    return (*mVertices)[i];
}

Vector3ui SurfacePointerAccess::getTriangle(uint i) {
    return (*mTriangles)[i];
}

std::vector<MeshVertex> SurfacePointerAccess::getVertices() {
    std::vector<MeshVertex> copy = *mVertices;
    return copy;
}

std::vector<Vector3ui> SurfacePointerAccess::getTriangles() {
    std::vector<Vector3ui> copy = *mTriangles;
    return copy;
}

} // end namespace fast


