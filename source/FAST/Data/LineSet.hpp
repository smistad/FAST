#ifndef LINESET_HPP_
#define LINESET_HPP_

#include "SpatialDataObject.hpp"
#include "DynamicData.hpp"
#include "FAST/Data/Access/LineSetAccess.hpp"

namespace fast {

class LineSet : public SpatialDataObject {
    FAST_OBJECT(LineSet);
    public:
        void create(std::vector<Vector3f> vertices, std::vector<Vector2ui> lines);
        LineSetAccess::pointer getAccess(accessType access);
        BoundingBox getBoundingBox() const;
        ~LineSet();
    private:
        LineSet();
        void freeAll();
        void free(ExecutionDevice::pointer device);

        std::vector<Vector3f> mVertices;
        std::vector<Vector2ui> mLines;

        // Necessary to give LineSetAccess access to the accessFinished method
        friend class LineSetAccess;
};

}



#endif
