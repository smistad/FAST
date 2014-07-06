#ifndef SCENEGRAPH_HPP_
#define SCENEGRAPH_HPP_

#include "DataObject.hpp"

namespace fast {

class SceneGraphNode {
    FAST_OBJECT(SceneGraphNode)
    public:
        void setDataObject(DataObject::pointer data);
        //void setTransformation();
    private:
        SceneGraphNode();
        DataObject::pointer mData;
        bool mIsDataNode;

        // LinearTransformation transformation

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
    private:
        SceneGraph();
        SceneGraph(SceneGraph const&); // Don't implement
        void operator=(SceneGraph const&); // Don't implement

        std::vector<SceneGraphNode::pointer> mNodes;
};

} // end namespace fast



#endif /* SCENEGRAPH_HPP_ */
