#include "MeshAccess.hpp"
#include "FAST/Data/Mesh.hpp"

namespace fast {

MeshAccess::MeshAccess(
        std::vector<float>* coordinates,
        std::vector<float>* normals,
        std::vector<float>* colors,
        std::vector<uint>* lines,
        std::vector<uint>* triangles,
        SharedPointer<Mesh> mesh) {

    mCoordinates = coordinates;
    mNormals = normals;
    mColors = colors;
    mLines = lines;
    mTriangles = triangles;
    mMesh = mesh;
}

void MeshAccess::release() {
	mMesh->accessFinished();
}

MeshAccess::~MeshAccess() {
	release();
}

void MeshAccess::setVertex(uint i, MeshVertex vertex) {
    Vector3f pos = vertex.getPosition();
    (*mCoordinates)[i*3] = pos[0];
    (*mCoordinates)[i*3+1] = pos[1];
    (*mCoordinates)[i*3+2] = pos[2];
    Vector3f normal = vertex.getNormal();
    (*mNormals)[i*3] = normal[0];
    (*mNormals)[i*3+1] = normal[1];
    (*mNormals)[i*3+2] = normal[2];
    Color color = vertex.getColor();
    (*mColors)[i*3] = color.getRedValue();
    (*mColors)[i*3+1] = color.getGreenValue();
    (*mColors)[i*3+2] = color.getBlueValue();
}

MeshVertex MeshAccess::getVertex(uint i) {
    Color color((*mColors)[i*3], (*mColors)[i*3+1], (*mColors)[i*3+2]);
    Vector3f coordinate((*mCoordinates)[i*3], (*mCoordinates)[i*3+1], (*mCoordinates)[i*3+2]);
    Vector3f normal((*mNormals)[i*3], (*mNormals)[i*3+1], (*mNormals)[i*3+2]);
    return MeshVertex(coordinate, normal, color);
}

MeshTriangle MeshAccess::getTriangle(uint i) {
    MeshTriangle triangle((*mTriangles)[i*3], (*mTriangles)[i*3+1], (*mTriangles)[i*3+2]);
    return triangle;
}

void MeshAccess::setLine(uint i, MeshLine line) {
    (*mLines)[i*2] = line.getEndpoint1();
    (*mLines)[i*2+1] = line.getEndpoint2();
}

MeshLine MeshAccess::getLine(uint i) {
    MeshLine line((*mLines)[i*2], (*mLines)[i*2+1]);
    return line;
}

void MeshAccess::setTriangle(uint i, MeshTriangle triangle) {
    (*mTriangles)[i*3] = triangle.getEndpoint1();
    (*mTriangles)[i*3+1] = triangle.getEndpoint2();
    (*mTriangles)[i*3+2] = triangle.getEndpoint3();
}

std::vector<MeshVertex> MeshAccess::getVertices() {
    std::vector<MeshVertex> vertex;
    for(uint i = 0; i < mCoordinates->size()/3; i++) {
        vertex.push_back(getVertex(i));
    }
    return vertex;
}

std::vector<MeshTriangle> MeshAccess::getTriangles() {
    std::vector<MeshTriangle> triangles;
    for(uint i = 0; i < mTriangles->size()/3; i++) {
        triangles.push_back(getTriangle(i));
    }
    return triangles;
}

std::vector<MeshLine> MeshAccess::getLines() {
    std::vector<MeshLine> lines;
    for(uint i = 0; i < mLines->size()/2; i++) {
        lines.push_back(getLine(i));
    }
    return lines;
}

void MeshAccess::addVertex(MeshVertex v) {
    // Add dummy values
    mCoordinates->push_back(0);
    mCoordinates->push_back(0);
    mCoordinates->push_back(0);
    mNormals->push_back(0);
    mNormals->push_back(0);
    mNormals->push_back(0);
    mColors->push_back(0);
    mColors->push_back(0);
    mColors->push_back(0);
    setVertex(mCoordinates->size()/3 - 1, v);
}

void MeshAccess::addTriangle(MeshTriangle t) {
    mTriangles->push_back(t.getEndpoint1());
    mTriangles->push_back(t.getEndpoint2());
    mTriangles->push_back(t.getEndpoint3());
}

void MeshAccess::addLine(MeshLine l) {
    mLines->push_back(l.getEndpoint1());
    mLines->push_back(l.getEndpoint2());
}

} // end namespace fast


