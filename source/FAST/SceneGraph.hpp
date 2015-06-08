#ifndef SCENEGRAPH_HPP_
#define SCENEGRAPH_HPP_

#include "FAST/AffineTransformation.hpp"
#include "FAST/Object.hpp"

namespace fast {


class SceneGraphNode : public Object {
    FAST_OBJECT(SceneGraphNode)
    public:
        void setTransformation(AffineTransformation transformation);
        void setParent(SceneGraphNode::pointer parent);
        SceneGraphNode::pointer getParent() const;
        AffineTransformation getTransformation() const;
        void reset();
        bool isDataNode() const;
        bool isRootNode() const;
    private:
        SceneGraphNode();

        SceneGraphNode::pointer mParent;
        bool mIsRootNode;
        AffineTransformation mTransformation;
};

class SpatialDataObject;

namespace SceneGraph {
    AffineTransformation getAffineTransformationBetweenNodes(SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB);
    AffineTransformation getAffineTransformationFromNode(SceneGraphNode::pointer node);
    AffineTransformation getAffineTransformationFromData(SharedPointer<SpatialDataObject> node);
    void setParentNode(SharedPointer<SpatialDataObject> child, SharedPointer<SpatialDataObject> parent);
    SceneGraphNode::pointer insertParentNodeToData(SharedPointer<SpatialDataObject> child, AffineTransformation transform);
    SceneGraphNode::pointer insertParentNodeToNode(SceneGraphNode::pointer child, AffineTransformation transform);
};

} // end namespace fast



#endif /* SCENEGRAPH_HPP_ */
