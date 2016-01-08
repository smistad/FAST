#include "Camera.hpp"

namespace fast {


void Camera::create(Vector3f position, Vector3f upVector,
        Vector3f target) {
    mPosition = position;
    mUpVector = upVector;
    mTarget = target;
}

CameraAccess::pointer Camera::getAccess(accessType accessType) {
    CameraAccess::pointer accessObject(new CameraAccess(&mPosition, &mUpVector, &mTarget));

    return accessObject;
}

Camera::Camera() {
    mPosition = Vector3f(0,0,0);
    mUpVector = Vector3f(0,0,0);
    mTarget = Vector3f(0,0,0);
}

void Camera::freeAll() {
}

void Camera::free(ExecutionDevice::pointer device) {
}

} // end namespace fast
