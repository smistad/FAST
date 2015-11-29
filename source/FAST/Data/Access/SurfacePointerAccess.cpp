#include "SurfacePointerAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

SurfacePointerAccess::SurfacePointerAccess(
        std::vector<SurfaceVertex>* vertices,
        std::vector<Vector3ui>* triangles,
        SharedPointer<Mesh> mesh) {
    mVertices = vertices;
    mTriangles = triangles;
    mMesh = mesh;;
}

void SurfacePointerAccess::release() {
	mMesh->hostAccessFinished();
}

SurfacePointerAccess::~SurfacePointerAccess() {
	release();
}


SurfaceVertex SurfacePointerAccess::getVertex(uint i) {
    return (*mVertices)[i];
}

Vector3ui SurfacePointerAccess::getTriangle(uint i) {
    return (*mTriangles)[i];
}

std::vector<SurfaceVertex> SurfacePointerAccess::getVertices() {
    std::vector<SurfaceVertex> copy = *mVertices;
    return copy;
}

std::vector<Vector3ui> SurfacePointerAccess::getTriangles() {
    std::vector<Vector3ui> copy = *mTriangles;
    return copy;
}

} // end namespace fast


