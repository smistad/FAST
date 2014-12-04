#ifndef POINT_SET_HPP_
#define POINT_SET_HPP_

#include "DataObject.hpp"
#include "DynamicData.hpp"
#include "PointSetAccess.hpp"

namespace fast {

class PointSetData : public virtual DataObject {
    public:
        typedef SharedPointer<PointSetData> pointer;
        virtual ~PointSetData() {};
};


class PointSet : public virtual DataObject {
    FAST_OBJECT(PointSet)
    public:
        void create(std::vector<Vector3f> points);
        uint getNrOfPoints() const;
        PointSetAccess getAccess(accessType access);
        BoundingBox getBoundingBox() const;
        ~PointSet();
    private:
        PointSet();
        void freeAll();
        void free(ExecutionDevice::pointer device);
        bool isAnyDataBeingAccessed() const;

        // Host data
        std::vector<Vector3f> mPointSet;
        bool mHostDataIsBeingAccessed;

        bool mIsBeingWrittenTo;
};

class DynamicPointSet : public PointSetData, public DynamicData<PointSet> {
    FAST_OBJECT(DynamicPointSet)
};


} // end namespace fast

#endif
