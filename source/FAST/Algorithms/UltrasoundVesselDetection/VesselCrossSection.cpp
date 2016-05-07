#include "VesselCrossSection.hpp"

namespace fast {

VesselCrossSection::VesselCrossSection() {
    mMajorRadius = -1.0f;
    mMinorRadius = -1.0f;
}

void VesselCrossSection::create(Vector3f globalCenterPosition,
        Vector2f imageCenterPosition, float majorRadius, float minorRadius) {
    mGlobalCenterPosition = globalCenterPosition;
    mImageCenterPosition = imageCenterPosition;
    mMajorRadius = majorRadius;
    mMinorRadius = minorRadius;
    updateModifiedTimestamp();
}

VesselCrossSectionAccess::pointer VesselCrossSection::getAccess(
        accessType access) {
    // TODO improve this
    VesselCrossSectionAccess::pointer accessObject(new
            VesselCrossSectionAccess(
                    &mGlobalCenterPosition,
                    &mImageCenterPosition,
                    &mMajorRadius,
                    &mMinorRadius));

    return accessObject;
}

void VesselCrossSection::freeAll() {
}

void VesselCrossSection::free(ExecutionDevice::pointer device) {
}


} // end namespace fast
