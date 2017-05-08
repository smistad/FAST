#include "MeshAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

MeshAccess::MeshAccess(
        std::vector<MeshVertex>* vertices,
        std::vector<MeshLine>* lines,
        std::vector<MeshTriangle>* triangles,
        SharedPointer<Mesh> mesh) {
    mVertices = vertices;
    mLines = lines;
    mTriangles = triangles;
    mMesh = mesh;;
}

void MeshAccess::release() {
	mMesh->accessFinished();
}

MeshAccess::~MeshAccess() {
	release();
}

void MeshAccess::setVertex(uint i, MeshVertex vertex) {
    (*mVertices)[i] = vertex;
}

MeshVertex MeshAccess::getVertex(uint i) {
    return (*mVertices)[i];
}

MeshTriangle MeshAccess::getTriangle(uint i) {
    return (*mTriangles)[i];
}

void MeshAccess::setLine(uint i, MeshLine line) {
    (*mLines)[i] = line;
}

MeshLine MeshAccess::getLine(uint i) {
    return (*mLines)[i];
}

void MeshAccess::setTriangle(uint i, MeshTriangle triangle) {
    (*mTriangles)[i] = triangle;
}

std::vector<MeshVertex> MeshAccess::getVertices() {
    std::vector<MeshVertex> copy = *mVertices;
    return copy;
}

std::vector<MeshTriangle> MeshAccess::getTriangles() {
    std::vector<MeshTriangle> copy = *mTriangles;
    return copy;
}
std::vector<MeshLine> MeshAccess::getLines() {
    std::vector<MeshLine> copy = *mLines;
    return copy;
}

void MeshAccess::addVertex(MeshVertex v) {
    mVertices->push_back(v);
}

void MeshAccess::addTriangle(MeshTriangle t) {
    mTriangles->push_back(t);
}

void MeshAccess::addLine(MeshLine l) {
    mLines->push_back(l);
}

} // end namespace fast


