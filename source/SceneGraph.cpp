#include "SceneGraph.hpp"
#include "SpatialDataObject.hpp"

namespace fast {

void SceneGraphNode::reset() {
    // Remove previous parent
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    setParent(newRootNode);
    // Set transformation to identity
    setTransformation(LinearTransformation());
}

// Set transformation to its parent
void SceneGraphNode::setTransformation(
        LinearTransformation transformation) {
    mTransformation = transformation;
    mIsRootNode = false;
}

void SceneGraphNode::setParent(SceneGraphNode::pointer parent) {
    mParent = parent;
    mIsRootNode = false;
}

bool SceneGraphNode::isDataNode() const {
    return !mIsRootNode;
}

SceneGraphNode::pointer SceneGraphNode::getParent() const {
    if(mIsRootNode)
        throw Exception("Can't getParent from a SceneGraphNode that is a root node");
    return mParent;
}

bool SceneGraphNode::isRootNode() const {
    return mIsRootNode;
}

LinearTransformation SceneGraphNode::getLinearTransformation() const {
    return mTransformation;
}

SceneGraphNode::SceneGraphNode() {
    mIsRootNode = true;
}

LinearTransformation SceneGraph::getLinearTransformationBetweenNodes(
        SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB) {
    // TODO traverse the graph from node A to node B

			return LinearTransformation();
}

LinearTransformation SceneGraph::getLinearTransformationFromNode(
        SceneGraphNode::pointer node) {
    SceneGraphNode::pointer currentNode = node;
    LinearTransformation transformation;
    while(!currentNode->isRootNode()) {
        transformation = currentNode->getLinearTransformation()*transformation;
        currentNode = currentNode->getParent();
    }

    return transformation;
}

LinearTransformation SceneGraph::getLinearTransformationFromData(
        SpatialDataObject::pointer data) {
    SceneGraphNode::pointer currentNode = data->getSceneGraphNode();
    return SceneGraph::getLinearTransformationFromNode(currentNode);
}

void SceneGraph::setParentNode(SpatialDataObject::pointer child,
        SpatialDataObject::pointer parent) {
    SceneGraphNode::pointer thisNode = child->getSceneGraphNode();
    SceneGraphNode::pointer parentNode = parent->getSceneGraphNode();
    thisNode->setParent(parentNode);
}


SceneGraphNode::pointer SceneGraph::insertParentNodeToData(SpatialDataObject::pointer child, LinearTransformation transform) {
    SceneGraphNode::pointer childNode = child->getSceneGraphNode();
    return insertParentNodeToNode(childNode, transform);
}

SceneGraphNode::pointer SceneGraph::insertParentNodeToNode(SceneGraphNode::pointer childNode, LinearTransformation transform) {
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setTransformation(childNode->getLinearTransformation());
    newNode->setParent(childNode->getParent());
    childNode->setTransformation(transform);
    childNode->setParent(newNode);
    return newNode;
}

} // end namespace fast
