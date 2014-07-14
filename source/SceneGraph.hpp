#ifndef SCENEGRAPH_HPP_
#define SCENEGRAPH_HPP_

#include "DataObject.hpp"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/unordered_set.hpp>

namespace fast {

class LinearTransformation : public boost::numeric::ublas::matrix<float> {
    public:
        LinearTransformation();
        // Copy
        LinearTransformation(boost::numeric::ublas::matrix<float> m) : boost::numeric::ublas::matrix<float>(m) {};
        LinearTransformation getInverse();
        LinearTransformation operator*(const LinearTransformation &other);
        boost::numeric::ublas::matrix<float> getMatrix() const;
};

class SceneGraphNode : public Object {
    FAST_OBJECT(SceneGraphNode)
    public:
        void setDataObject(WeakPointer<DataObject> data);
        void setTransformation(LinearTransformation transformation);
        void setParent(SceneGraphNode::pointer parent);
        WeakPointer<DataObject> getData() const;
        SceneGraphNode::pointer getParent() const;
        LinearTransformation getLinearTransformation() const;
        bool isDataNode() const;
        bool isRootNode() const;
    private:
        SceneGraphNode();

        SceneGraphNode::pointer mParent;
        WeakPointer<DataObject> mData;
        bool mIsDataNode;
        bool mIsRootNode;
        LinearTransformation mTransformation;
};

class SceneGraph {
    public:
        static SceneGraph& getInstance();
        SceneGraphNode::pointer addDataNode(WeakPointer<DataObject> data, SceneGraphNode::pointer parent);
        SceneGraphNode::pointer addDataNode(DataObject::pointer data, SceneGraphNode::pointer parent);
        SceneGraphNode::pointer addDataNodeToNewRoot(WeakPointer<DataObject> data);
        SceneGraphNode::pointer addDataNodeToNewRoot(DataObject::pointer data);
        SceneGraphNode::pointer addNode(SceneGraphNode::pointer parent);
        SceneGraphNode::pointer getDataNode(WeakPointer<DataObject> data);
        void removeDataNode(WeakPointer<DataObject> data);
        void removeNode(SceneGraphNode::pointer node);
        LinearTransformation getLinearTransformationBetweenNodes(SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB);
        LinearTransformation getLinearTransformationFromNode(SceneGraphNode::pointer node);
        void deleteGraph();
    private:
        SceneGraph();
        SceneGraph(SceneGraph const&); // Don't implement
        void operator=(SceneGraph const&); // Don't implement

        boost::unordered_set<SceneGraphNode::pointer> mNodes;
        boost::unordered_map<WeakPointer<DataObject>, SceneGraphNode::pointer> mDataToNodesMap;
};

} // end namespace fast



#endif /* SCENEGRAPH_HPP_ */
