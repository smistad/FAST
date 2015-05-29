#include "PointSetAccess.hpp"

namespace fast {

PointSetAccess::PointSetAccess(std::vector<Vector3f>* points, bool* accessFlag,
        bool* accessFlag2) {
    mPointSet = points;
    mAccessFlag = accessFlag;
    mAccessFlag2 = accessFlag2;
}

Vector3f PointSetAccess::getPoint(uint i) const {
    return mPointSet->at(i);
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
    *mAccessFlag = false;
    *mAccessFlag2 = false;
}

PointSetAccess::~PointSetAccess() {
    *mAccessFlag = false;
    *mAccessFlag2 = false;
}

} // end namespace fast
