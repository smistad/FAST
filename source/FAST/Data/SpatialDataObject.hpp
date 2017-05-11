#ifndef SPATIAL_DATA_OBJECT_HPP_
#define SPATIAL_DATA_OBJECT_HPP_

#include "FAST/Data/DataObject.hpp"
#include "BoundingBox.hpp"
#include "FAST/SceneGraph.hpp"

namespace fast {

class FAST_EXPORT  SpatialDataObject : public DataObject {
    public:
        typedef SharedPointer<SpatialDataObject> pointer;
        SpatialDataObject();
        virtual BoundingBox getBoundingBox() const;
        virtual BoundingBox getTransformedBoundingBox() const;
        SceneGraphNode::pointer getSceneGraphNode() const;
        static std::string getStaticNameOfClass() {
            return "";
        };
    protected:
        BoundingBox mBoundingBox;
    private:
        SceneGraphNode::pointer mSceneGraphNode;

};

}

#endif
