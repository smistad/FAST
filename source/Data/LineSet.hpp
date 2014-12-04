#ifndef LINESET_HPP_
#define LINESET_HPP_

#include "DataObject.hpp"
#include "DynamicData.hpp"
#include "Access.hpp"

namespace fast {

class LineSetData : public virtual DataObject {
    public:
        typedef SharedPointer<LineSetData> pointer;
        virtual ~LineSetData() {};
};

class LineSetAccess;
class LineSet : public LineSetData {
    FAST_OBJECT(LineSet);
    public:
        void create(std::vector<Vector3f> vertices, std::vector<Vector2ui> lines);
        LineSetAccess getAccess(accessType access);
        BoundingBox getBoundingBox() const;
        ~LineSet();
    private:
        LineSet();
        void freeAll();
        void free(ExecutionDevice::pointer device);
        bool isAnyDataBeingAccessed() const;

        std::vector<Vector3f> mVertices;
        std::vector<Vector2ui> mLines;
        bool mHostDataIsBeingAccessed;

        bool mIsBeingWrittenTo;
};

class DynamicLineSet : public LineSetData, public DynamicData<LineSet> {
    FAST_OBJECT(DynamicLineSet)
};

}



#endif
