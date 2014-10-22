#include "SurfacePointerAccess.hpp"

namespace fast {

SurfacePointerAccess::SurfacePointerAccess(
        std::vector<SurfaceVertex>* vertices,
        std::vector<Uint3>* triangles,
        bool* accessFlag,
        bool* accessFlag2) {
    mVertices = vertices;
    mTriangles = triangles;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

void SurfacePointerAccess::release() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
}

SurfacePointerAccess::~SurfacePointerAccess() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
}


SurfaceVertex SurfacePointerAccess::getVertex(uint i) {
    return (*mVertices)[i];
}

Uint3 SurfacePointerAccess::getTriangle(uint i) {
    return (*mTriangles)[i];
}

std::vector<SurfaceVertex> SurfacePointerAccess::getVertices() {
    std::vector<SurfaceVertex> copy = *mVertices;
    return copy;
}

std::vector<Uint3> SurfacePointerAccess::getTriangles() {
    std::vector<Uint3> copy = *mTriangles;
    return copy;
}

} // end namespace fast


