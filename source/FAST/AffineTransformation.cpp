#include "FAST/AffineTransformation.hpp"

namespace fast {

/**
 * Initializes linear transformation object to identity matrix
 */
AffineTransformation::AffineTransformation() {
    mTransform.matrix() = Eigen::Matrix4f::Identity();
}

AffineTransformation::AffineTransformation(const Eigen::Affine3f& transform) {
    mTransform.matrix() = transform.matrix();
}

AffineTransformation& AffineTransformation::operator=(const Eigen::Affine3f& transform) {
    mTransform.matrix() = transform.matrix();
    return *this;
}

Vector3f AffineTransformation::getEulerAngles() const {
    Matrix3f rotationMatrix = mTransform.rotation();
    return rotationMatrix.eulerAngles(0, 1, 2);
}

Affine3f AffineTransformation::getTransform() const {
    return mTransform;
}

AffineTransformation::pointer AffineTransformation::multiply(AffineTransformation::pointer transformation) {
	AffineTransformation::pointer result = AffineTransformation::New();
	result->getTransform().matrix() = mTransform.matrix()*transformation->getTransform().matrix();
	return result;
}

Vector3f AffineTransformation::multiply(Vector3f point) {
	return (mTransform.matrix()*point.homogeneous()).head(3);
}



}
