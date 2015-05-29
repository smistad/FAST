#include "SpatialDataObject.hpp"
#include "FAST/AffineTransformation.hpp"

namespace fast {

SpatialDataObject::SpatialDataObject() {
    // Create scene graph node and attach it to a new root
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    SceneGraphNode::pointer node = SceneGraphNode::New();
    node->setParent(newRootNode);
    mSceneGraphNode = node;
}

SceneGraphNode::pointer SpatialDataObject::getSceneGraphNode() const {
    return mSceneGraphNode;
}

BoundingBox SpatialDataObject::getBoundingBox() const {
    return mBoundingBox;
}

BoundingBox SpatialDataObject::getTransformedBoundingBox() const {
    AffineTransformation T = SceneGraph::getAffineTransformationFromData(DataObject::pointer(mPtr.lock()));

    return getBoundingBox().getTransformedBoundingBox(T);
}


}
