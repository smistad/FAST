#pragma once

#include <FAST/Data/DataBoundingBox.hpp>
#include <FAST/Object.hpp>

namespace fast {


class FAST_EXPORT  SceneGraphNode : public Object {
    FAST_OBJECT(SceneGraphNode)
    public:
        void setTransform(Affine3f transformation);
        void setTransform(Transform::pointer transformation);
        void setParent(SceneGraphNode::pointer parent);
        SceneGraphNode::pointer getParent() const;
        Transform::pointer getTransform() const;
        void reset();
        bool isDataNode() const;
        bool isRootNode() const;
    private:
        SceneGraphNode();

        SceneGraphNode::pointer mParent;
        bool mIsRootNode;
        Transform::pointer mTransformation;
};

class SpatialDataObject;

namespace SceneGraph {
	FAST_EXPORT Transform::pointer getTransformBetweenNodes(SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB);
	FAST_EXPORT Transform::pointer getTransformFromNode(SceneGraphNode::pointer node);
	FAST_EXPORT Transform::pointer getTransformFromData(std::shared_ptr<SpatialDataObject> node);
    FAST_EXPORT Affine3f getEigenTransformFromNode(SceneGraphNode::pointer node);
	FAST_EXPORT Affine3f getEigenTransformFromData(std::shared_ptr<SpatialDataObject> node);
	FAST_EXPORT void setParentNode(std::shared_ptr<SpatialDataObject> child, std::shared_ptr<SpatialDataObject> parent);
	FAST_EXPORT SceneGraphNode::pointer insertParentNodeToData(std::shared_ptr<SpatialDataObject> child, Transform::pointer transform);
	FAST_EXPORT SceneGraphNode::pointer insertParentNodeToNode(SceneGraphNode::pointer child, Transform::pointer transform);
};

} // end namespace fast

