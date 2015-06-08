#include "FAST/AffineTransformation.hpp"

namespace fast {

/**
 * Initializes linear transformation object to identity matrix
 */
AffineTransformation::AffineTransformation() {
    matrix() = Eigen::Matrix4f::Identity();
}

AffineTransformation::AffineTransformation(const Eigen::Affine3f& transform) {
    matrix() = transform.matrix();
}

AffineTransformation& AffineTransformation::operator=(const Eigen::Affine3f& transform) {
    matrix() = transform.matrix();
    return *this;
}

Vector3f AffineTransformation::getEulerAngles() const {
    Matrix3f rotationMatrix = rotation();
    return rotationMatrix.eulerAngles(0, 1, 2);
}



}
