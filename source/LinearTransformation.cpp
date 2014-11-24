#include "LinearTransformation.hpp"

namespace fast {

/**
 * Initializes linear transformation object to identity matrix
 */
LinearTransformation::LinearTransformation() {
    mTransform = Eigen::Transform<float, 3, Eigen::Affine>::Identity();
}

LinearTransformation LinearTransformation::getInverse() {
    LinearTransformation inverse;
    inverse.setTransform(mTransform.inverse());
    return inverse;
}

Eigen::Transform<float, 3, Eigen::Affine> LinearTransformation::getTransform() const {
    return mTransform;
}

void LinearTransformation::setTransform(const Eigen::Transform<float, 3, Eigen::Affine> transform) {
    mTransform = transform;
}

LinearTransformation LinearTransformation::operator *(
        LinearTransformation other) {
    LinearTransformation mult;
    mult.setTransform(mTransform*other.getTransform());
    return mult;
}

Vector3f LinearTransformation::operator*(Vector3f vertex) const {
    return mTransform*vertex.homogeneous();
}

float& LinearTransformation::operator()(uint i, uint j) {
    return mTransform(i,j);
}

Vector3f LinearTransformation::getEulerAngles() const {
    Matrix3f rotationMatrix = mTransform.rotation();
    return rotationMatrix.eulerAngles(0, 1, 2);
}

}

