#include "CameraAccess.hpp"

namespace fast {


CameraAccess::CameraAccess(Vector3f* position, Vector3f* upVector,
        Vector3f* target) {
    mPosition = position;
    mUpVector = upVector;
    mTarget = target;
}

Eigen::Affine3f CameraAccess::getCameraTransformation() const {
    // Equations based on gluLookAt https://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml
    Vector3f F = (*mTarget) - (*mPosition);
    F.normalize();
    Vector3f up = *mUpVector;
    up.normalize();
    Vector3f s = F.cross(up);
    Vector3f sNormalized = s;
    sNormalized.normalize();
    Vector3f u = sNormalized.cross(F);

    Matrix3f M;
    // First row
    M(0,0) = s[0];
    M(0,1) = s[1];
    M(0,2) = s[2];
    // Second row
    M(1,0) = u[0];
    M(1,1) = u[1];
    M(1,2) = u[2];
    // Third row
    M(2,0) = -F[0];
    M(2,1) = -F[1];
    M(2,2) = -F[2];

    Eigen::Affine3f transformation = Eigen::Affine3f::Identity();
    transformation.rotate(M);
    transformation.translate(-(*mPosition));

    return transformation;
}

Vector3f CameraAccess::getTargetPosition() const {
    return *mTarget;
}

Vector3f CameraAccess::getPosition() const {
    return *mPosition;
}

Vector3f CameraAccess::getUpVector() const {
    return *mUpVector;
}

void CameraAccess::setTargetPosition(Vector3f position) {
    *mTarget = position;
}

void CameraAccess::setPosition(Vector3f position) {
    *mPosition = position;
}

void CameraAccess::setUpVector(Vector3f upVector) {
    *mUpVector = upVector;
}

} // end namespace fast
