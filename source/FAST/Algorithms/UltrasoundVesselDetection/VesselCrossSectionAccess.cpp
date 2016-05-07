#include "VesselCrossSectionAccess.hpp"

namespace fast {

VesselCrossSectionAccess::VesselCrossSectionAccess(
        Vector3f* globalCenterPosition, Vector2f* imageCenterPosition,
        float* majorRadius, float* minorRadius) {
    mGlobalCenterPosition = globalCenterPosition;
    mImageCenterPosition = imageCenterPosition;
    mMajorRadius = majorRadius;
    mMinorRadius = minorRadius;
}

float VesselCrossSectionAccess::getMajorRadius() const {
    return *mMajorRadius;
}

float VesselCrossSectionAccess::getMinorRadius() const {
    return *mMinorRadius;
}

Vector3f VesselCrossSectionAccess::getGlobalCenterPosition() const {
    return *mGlobalCenterPosition;
}

Vector2f VesselCrossSectionAccess::getImageCenterPosition() const {
    return *mImageCenterPosition;
}

void VesselCrossSectionAccess::setMajorRadius(float radius) {
    *mMajorRadius = radius;
}

void VesselCrossSectionAccess::setMinorRadius(float radius) {
    *mMinorRadius = radius;
}

void VesselCrossSectionAccess::setGlobalCenterPosition(
        Vector3f position) {
    *mGlobalCenterPosition = position;
}

void VesselCrossSectionAccess::setImageCenterPosition(Vector2f position) {
    *mImageCenterPosition = position;
}

}
