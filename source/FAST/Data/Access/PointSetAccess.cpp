#include "PointSetAccess.hpp"
#include "FAST/Data/PointSet.hpp"

namespace fast {

PointSetAccess::PointSetAccess(std::vector<Vector3f>* points, PointSet::pointer object) {
    mPointSet = points;
    mObject = object;
}

Vector3f PointSetAccess::getPoint(uint i) const {
    return mPointSet->at(i);
}

std::vector<Vector3f> PointSetAccess::getPoints() const {
	return *mPointSet;
}

void PointSetAccess::setPoint(uint i, const Vector3f point) {
    mPointSet->at(i) = point;
}

void PointSetAccess::addPoint(const Vector3f point) {
    mPointSet->push_back(point);
}

MatrixXf PointSetAccess::getPointSetAsMatrix() const {
    MatrixXf matrix = MatrixXf::Constant(3, mPointSet->size(), 0);

    for(uint i = 0; i < mPointSet->size(); i++) {
        matrix.col(i) = mPointSet->operator[](i);
    }

    return matrix;
}

void PointSetAccess::release() {
	mObject->accessFinished();
}

PointSetAccess::~PointSetAccess() {
	release();
}

} // end namespace fast
