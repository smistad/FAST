#ifndef VESSEL_CROSS_SECTION_HPP_
#define VESSEL_CROSS_SECTION_HPP_

#include "FAST/Data/DataObject.hpp"
#include "VesselCrossSectionAccess.hpp"
#include "FAST/Data/Access/Access.hpp"

namespace fast {

class FAST_EXPORT  VesselCrossSection : public DataObject {
    FAST_OBJECT(VesselCrossSection)
    public:
        void create(Vector3f globalCenterPosition, Vector2f mImageCenterPosition, float mMajorRadius, float mMinorRadius);
        VesselCrossSectionAccess::pointer getAccess(accessType access);
    private:
        VesselCrossSection();
        Vector3f mGlobalCenterPosition;
        Vector2f mImageCenterPosition;
        float mMajorRadius;
        float mMinorRadius;

        void freeAll();
        void free(ExecutionDevice::pointer device);
};

} // end namespace fast


#endif
