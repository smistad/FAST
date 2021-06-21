#pragma once

#include "FAST/Data/DataObject.hpp"
#include "DataBoundingBox.hpp"
#include "FAST/SceneGraph.hpp"

namespace fast {

class FAST_EXPORT  SpatialDataObject : public DataObject {
    public:
        typedef std::shared_ptr<SpatialDataObject> pointer;
        SpatialDataObject();
        virtual DataBoundingBox getBoundingBox() const;
        virtual DataBoundingBox getTransformedBoundingBox() const;
        SceneGraphNode::pointer getSceneGraphNode() const;
        static std::string getStaticNameOfClass() {
            return "";
        };
    protected:
        DataBoundingBox mBoundingBox;
    private:
        SceneGraphNode::pointer mSceneGraphNode;

};

}
