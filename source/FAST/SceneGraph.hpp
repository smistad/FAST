#ifndef SCENEGRAPH_HPP_
#define SCENEGRAPH_HPP_

#include "FAST/AffineTransformation.hpp"
#include "FAST/Object.hpp"

namespace fast {


class FAST_EXPORT  SceneGraphNode : public Object {
    FAST_OBJECT(SceneGraphNode)
    public:
        void setTransformation(AffineTransformation::pointer transformation);
        void setParent(SceneGraphNode::pointer parent);
        SceneGraphNode::pointer getParent() const;
        AffineTransformation::pointer getTransformation() const;
        void reset();
        bool isDataNode() const;
        bool isRootNode() const;
    private:
        SceneGraphNode();

        SceneGraphNode::pointer mParent;
        bool mIsRootNode;
        AffineTransformation::pointer mTransformation;
};

class SpatialDataObject;

namespace SceneGraph {
	FAST_EXPORT AffineTransformation::pointer getAffineTransformationBetweenNodes(SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB);
	FAST_EXPORT AffineTransformation::pointer getAffineTransformationFromNode(SceneGraphNode::pointer node);
	FAST_EXPORT AffineTransformation::pointer getAffineTransformationFromData(SharedPointer<SpatialDataObject> node);
	FAST_EXPORT Affine3f getEigenAffineTransformationFromData(SharedPointer<SpatialDataObject> node);
	FAST_EXPORT void setParentNode(SharedPointer<SpatialDataObject> child, SharedPointer<SpatialDataObject> parent);
	FAST_EXPORT SceneGraphNode::pointer insertParentNodeToData(SharedPointer<SpatialDataObject> child, AffineTransformation::pointer transform);
	FAST_EXPORT SceneGraphNode::pointer insertParentNodeToNode(SceneGraphNode::pointer child, AffineTransformation::pointer transform);
};

} // end namespace fast



#endif /* SCENEGRAPH_HPP_ */
