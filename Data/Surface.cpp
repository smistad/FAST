#include "Surface.hpp"

namespace fast {

void Surface::create(
        std::vector<Float<3> > vertices,
        std::vector<Float<3> > normals,
        std::vector<Uint<3> > triangles) {
}

void Surface::create(unsigned int nrOfTriangles) {
}

VertexBufferObjectAccess Surface::getVertexBufferObjectAccess(
        accessType access,
        ExecutionDevice::pointer device) {
}

SurfacePointerAccess Surface::getSurfacePointerAccess(accessType access) {
}

Surface::~Surface() {
}

Surface::Surface() {
}

void Surface::freeAll() {
}

void Surface::free(ExecutionDevice::pointer device) {
}

} // end namespace fast
