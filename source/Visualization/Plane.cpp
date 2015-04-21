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

Vector3f Plane::getIntersectionPoint(Vector3f pointA, Vector3f pointB) const {

    Vector3f lineDirection = pointB - pointA;

    if(!mHasPosition)
        throw Exception("Can't calculate intersection point without a plane point set");

    if(lineDirection.dot(mNormal) == 0)
        throw Exception("The line and plane are parallel, can't calculate intersection point");

    // http://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    float d = (mPosition - pointA).dot(mNormal) / (lineDirection.dot(mNormal));

    Vector3f intersectionPoint =  pointA + d*lineDirection;

    if((pointB - intersectionPoint).dot(lineDirection) < 0) // Intersection point is not on the line
        throw Exception("The intersection point was not on the line.");

    return intersectionPoint;
}

bool Plane::hasPosition() const {
    return mHasPosition;
}

}
