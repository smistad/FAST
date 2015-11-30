#include "LineSetAccess.hpp"
#include "FAST/Data/LineSet.hpp"

namespace fast {

LineSetAccess::LineSetAccess(std::vector<Vector3f>* vertices,
        std::vector<Vector2ui>* lines, LineSet::pointer object) {
    mVertices = vertices;
    mLines = lines;
    mObject = object;
}

Vector3f LineSetAccess::getPoint(uint i) const {
    return mVertices->at(i);
}

void LineSetAccess::setPoint(uint i, const Vector3f point) {
    mVertices->at(i) = point;
}

void LineSetAccess::addPoint(const Vector3f point) {
    mVertices->push_back(point);
}

uint LineSetAccess::getNrOfPoints() const {
    return mVertices->size();
}

uint LineSetAccess::getNrOfLines() const {
    return mLines->size();
}

void LineSetAccess::addLine(uint i, uint j) {
    mLines->push_back(Vector2ui(i,j));
}

Vector2ui LineSetAccess::getLine(uint i) const {
    return mLines->at(i);
}

void LineSetAccess::deleteLine(uint i) {
    mLines->erase(mLines->begin() + i);
}

void LineSetAccess::release() {
	mObject->accessFinished();
}

LineSetAccess::~LineSetAccess() {
	release();
}


}
