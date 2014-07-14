#include "SceneGraph.hpp"
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>


namespace fast {



void SceneGraphNode::setDataObject(WeakPointer<DataObject> data) {
    mData = data;
    mIsDataNode = true;
}

void SceneGraphNode::setTransformation(
        LinearTransformation transformation) {
    mTransformation = transformation;
}

void SceneGraphNode::setParent(SceneGraphNode::pointer parent) {
    mParent = parent;
    mIsRootNode = false;
}

WeakPointer<DataObject> SceneGraphNode::getData() const {
    if(!mIsDataNode)
        throw Exception("SceneGraphNode is not a data node");
    return mData;
}

bool SceneGraphNode::isDataNode() const {
    return mIsDataNode;
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
    mIsDataNode = false;
    mIsRootNode = true;
}

SceneGraph& SceneGraph::getInstance() {
    static SceneGraph instance;
    return instance;
}

SceneGraphNode::pointer SceneGraph::addDataNode(WeakPointer<DataObject> data,
        SceneGraphNode::pointer parent) {
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setDataObject(data);
    newNode->setParent(parent);
    mNodes.insert(newNode);
    mDataToNodesMap[data] = newNode;
    return newNode;
}

SceneGraphNode::pointer SceneGraph::addDataNode(DataObject::pointer data,
        SceneGraphNode::pointer parent) {
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setDataObject(WeakPointer<DataObject>(data));
    newNode->setParent(parent);
    mNodes.insert(newNode);
    mDataToNodesMap[data] = newNode;
    return newNode;
}

SceneGraphNode::pointer SceneGraph::addDataNodeToNewRoot(
        WeakPointer<DataObject> data) {
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setDataObject(data);
    newNode->setParent(newRootNode);
    mNodes.insert(newRootNode);
    mNodes.insert(newNode);
    mDataToNodesMap[data] = newNode;
    return newNode;
}

SceneGraphNode::pointer SceneGraph::addDataNodeToNewRoot(
        DataObject::pointer data) {
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setDataObject(WeakPointer<DataObject>(data));
    newNode->setParent(newRootNode);
    mNodes.insert(newRootNode);
    mNodes.insert(newNode);
    mDataToNodesMap[data] = newNode;
    return newNode;
}

SceneGraphNode::pointer SceneGraph::addNode(
        SceneGraphNode::pointer parent) {
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setParent(parent);
    mNodes.insert(newNode);
    return newNode;
}

SceneGraphNode::pointer SceneGraph::getDataNode(
        WeakPointer<DataObject> data) {
    return mDataToNodesMap[data];
}

SceneGraphNode::pointer SceneGraph::getDataNode(
        DataObject::pointer data) {
    return mDataToNodesMap[WeakPointer<DataObject>(data)];
}

void SceneGraph::removeDataNode(WeakPointer<DataObject> data) {
    removeNode(getDataNode(data));
}

void SceneGraph::removeNode(SceneGraphNode::pointer node) {
    // Remove any data to node mappings first
    if(node->isDataNode())
        mDataToNodesMap.erase(node->getData());
    mNodes.erase(node);
}

LinearTransformation SceneGraph::getLinearTransformationBetweenNodes(
        SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB) {
    // TODO traverse the graph from node A to node B
}
LinearTransformation SceneGraph::getLinearTransformationFromNode(
        SceneGraphNode::pointer node) {
    SceneGraphNode::pointer currentNode = node;
    LinearTransformation transformation;
    while(!currentNode->isRootNode()) {
        transformation = transformation*currentNode->getLinearTransformation();
        currentNode = currentNode->getParent();
    }

    return transformation;
}

/**
 * Deletes the scene graph
 */
void SceneGraph::deleteGraph() {
    mNodes.clear();
    mDataToNodesMap.clear();
}

SceneGraph::SceneGraph() {
}



} // end namespace fast
