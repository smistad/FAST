#ifndef VESSEL_CROSS_SECTION_ACCESS_HPP_
#define VESSEL_CROSS_SECTION_ACCESS_HPP_

#include "FAST/SmartPointers.hpp"
#include "FAST/Data/DataTypes.hpp"

namespace fast {

class FAST_EXPORT  VesselCrossSectionAccess {
    public:
        VesselCrossSectionAccess(Vector3f* globalCenterPosition,
        Vector2f* imageCenterPosition, float* majorRadius, float* minorRadius);
        typedef UniquePointer<VesselCrossSectionAccess> pointer;
        float getMajorRadius() const;
        float getMinorRadius() const;
        Vector3f getGlobalCenterPosition() const;
        Vector2f getImageCenterPosition() const;
        void setMajorRadius(float radius);
        void setMinorRadius(float radius);
        void setGlobalCenterPosition(Vector3f position);
        void setImageCenterPosition(Vector2f position);
    private:
        Vector3f* mGlobalCenterPosition;
        Vector2f* mImageCenterPosition;
        float* mMajorRadius;
        float* mMinorRadius;
};

}

#endif
