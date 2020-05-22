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

DataBoundingBox SpatialDataObject::getBoundingBox() const {
    return mBoundingBox;
}

DataBoundingBox SpatialDataObject::getTransformedBoundingBox() const {
    AffineTransformation::pointer T = SceneGraph::getAffineTransformationFromNode(getSceneGraphNode());

    return getBoundingBox().getTransformedBoundingBox(T);
}


}
