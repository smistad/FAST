#ifndef SCENEGRAPH_HPP_
#define SCENEGRAPH_HPP_

#include "DataObject.hpp"
#include <boost/numeric/ublas/matrix.hpp>

namespace fast {

class LinearTransformation : public boost::numeric::ublas::matrix<float> {
    public:
        LinearTransformation() : boost::numeric::ublas::matrix(4,4) {};
};

class SceneGraphNode {
    FAST_OBJECT(SceneGraphNode)
    public:
        void setDataObject(DataObject::pointer data);
        void setTransformation(LinearTransformation transformation);
    private:
        SceneGraphNode();
        DataObject::pointer mData;
        bool mIsDataNode;

        LinearTransformation transformation;
};

class SceneGraph {
    public:
        static SceneGraph& getInstance();
        SceneGraphNode::pointer addDataNode(DataObject::pointer data, SceneGraphNode::pointer parent);
        SceneGraphNode::pointer addDataNodeToNewRoot(DataObject::pointer data);
        SceneGraphNode::pointer addNode(SceneGraphNode::pointer parent);
        SceneGraphNode::pointer getDataNode(DataObject::pointer data);
        void removeDataNode(SceneGraphNode::pointer node);
        void removeNode(SceneGraphNode::pointer node);
        LinearTransformation getLinearTransformationBetweenNodes(SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB);
        LinearTransformation getLinearTransformationFromNode(SceneGraphNode::pointer node);
    private:
        SceneGraph();
        SceneGraph(SceneGraph const&); // Don't implement
        void operator=(SceneGraph const&); // Don't implement

        std::vector<SceneGraphNode::pointer> mNodes;
};

} // end namespace fast



#endif /* SCENEGRAPH_HPP_ */
