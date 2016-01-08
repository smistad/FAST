#ifndef POINT_SET_HPP_
#define POINT_SET_HPP_

#include "SpatialDataObject.hpp"
#include "DynamicData.hpp"
#include "FAST/Data/Access/PointSetAccess.hpp"

namespace fast {

class PointSet : public SpatialDataObject {
    FAST_OBJECT(PointSet)
    public:
        void create(std::vector<Vector3f> points);
        uint getNrOfPoints() const;
        PointSetAccess::pointer getAccess(accessType access);
        BoundingBox getBoundingBox() const;
        ~PointSet();
    private:
        PointSet();
        void freeAll();
        void free(ExecutionDevice::pointer device);

        // Host data
        std::vector<Vector3f> mPointSet;

        // Necessary to give PointSetAccess access to the accessFinished method
        friend class PointSetAccess;
};

} // end namespace fast

#endif
