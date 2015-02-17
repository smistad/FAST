#ifndef SCENEGRAPH_HPP_
#define SCENEGRAPH_HPP_

#include "LinearTransformation.hpp"
#include "Object.hpp"

namespace fast {


class SceneGraphNode : public Object {
    FAST_OBJECT(SceneGraphNode)
    public:
        void setTransformation(LinearTransformation transformation);
        void setParent(SceneGraphNode::pointer parent);
        SceneGraphNode::pointer getParent() const;
        LinearTransformation getLinearTransformation() const;
        void reset();
        bool isDataNode() const;
        bool isRootNode() const;
    private:
        SceneGraphNode();

        SceneGraphNode::pointer mParent;
        bool mIsRootNode;
        LinearTransformation mTransformation;
};

class DataObject;

namespace SceneGraph {
    LinearTransformation getLinearTransformationBetweenNodes(SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB);
    LinearTransformation getLinearTransformationFromNode(SceneGraphNode::pointer node);
    LinearTransformation getLinearTransformationFromData(SharedPointer<DataObject> node);
    void setParentNode(SharedPointer<DataObject> child, SharedPointer<DataObject> parent);
    void insertParentNode(SharedPointer<DataObject> child, LinearTransformation transform);
};

} // end namespace fast



#endif /* SCENEGRAPH_HPP_ */
