#pragma once

#include "FAST/Data/DataObject.hpp"
#include "DataBoundingBox.hpp"
#include "FAST/SceneGraph.hpp"

namespace fast {

/**
 * @brief Abstract base class for all spatial data objects
 *
 * Spatial data objects are data which have a location in 2D/3D space.
 */
class FAST_EXPORT  SpatialDataObject : public DataObject {
    public:
        typedef std::shared_ptr<SpatialDataObject> pointer;
        SpatialDataObject();
        void setTransform(Transform::pointer transform, bool disconnectParentSceneGraphNode = false);
        Transform::pointer getTransform(bool getFullTransform = true);
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
