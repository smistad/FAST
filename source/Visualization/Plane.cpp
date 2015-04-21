#include "Plane.hpp"

namespace fast {


Plane::Plane(Vector3f normal) {
    mNormal = normal;
    mNormal.normalize();
    mHasPosition = false;
}

Plane::Plane(Vector3f normal, Vector3f position) {
    mNormal = normal;
    mNormal.normalize();
    mPosition = position;
    mHasPosition = true;
}

void Plane::setPosition(Vector3f position) {
    mPosition = position;
    mHasPosition = true;
}

void Plane::setNormal(Vector3f normal) {
    mNormal = normal;
    mNormal.normalize();
}

Vector3f Plane::getPosition() const {
    return mPosition;
}

Vector3f Plane::getNormal() const {
    return mNormal;
}

Plane Plane::Sagittal() {
    return Plane(Vector3f(1,0,0));
}

Plane Plane::Coronal() {
    return Plane(Vector3f(0,1,0));
}

Plane Plane::Axial() {
    return Plane(Vector3f(0,0,1));
}

}
