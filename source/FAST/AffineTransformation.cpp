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

AffineTransformation::pointer AffineTransformation::multiply(AffineTransformation::pointer transformation) {
	AffineTransformation::pointer result = AffineTransformation::New();
    Matrix4f transformMatrix = transformation->matrix();
    Vector4f t0 = transformMatrix.row(0);
    Vector4f t1 = transformMatrix.row(1);
    Vector4f t2 = transformMatrix.row(2);
    Vector4f t3 = transformMatrix.row(3);
    Matrix4f thisMatrix = matrix();
    Vector4f i0 = thisMatrix.row(0);
    Vector4f i1 = thisMatrix.row(1);
    Vector4f i2 = thisMatrix.row(2);
    Vector4f i3 = thisMatrix.row(3);
	result->matrix() = matrix()*transformation->matrix();
    Matrix4f resMatrix = result->matrix();
    i0 = resMatrix.row(0);
    i1 = resMatrix.row(1);
    i2 = resMatrix.row(2);
    i3 = resMatrix.row(3);
	return result;
}

Vector3f AffineTransformation::multiply(Vector3f point) {
	return (matrix()*point.homogeneous()).head(3);
}


AffineTransformation::pointer AffineTransformation::getInverse() {
	AffineTransformation::pointer result = AffineTransformation::New();
	result->matrix() = matrix().inverse();
	return result;
}

}
