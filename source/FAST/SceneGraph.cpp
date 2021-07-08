#include "FAST/SceneGraph.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

namespace fast {

void SceneGraphNode::reset() {
    // Remove previous parent
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    setParent(newRootNode);
    // Set transformation to identity
    setTransform(Transform::create());
}

// Set transformation to its parent
void SceneGraphNode::setTransform(
        Transform::pointer transformation) {
    mTransformation = transformation;
    mIsRootNode = false;
}

void SceneGraphNode::setTransform(
        Affine3f transformation) {
    mTransformation = Transform::create(transformation);
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

Transform::pointer SceneGraphNode::getTransform() const {
    return mTransformation;
}

SceneGraphNode::SceneGraphNode() {
    mIsRootNode = true;
    mTransformation = Transform::create();
}

Transform::pointer SceneGraph::getTransformBetweenNodes(
        SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB) {
    // TODO traverse the graph from node A to node B
    throw NotImplementedException();

    return Transform::create();
}

Transform::pointer SceneGraph::getTransformFromNode(
        SceneGraphNode::pointer node) {
    SceneGraphNode::pointer currentNode = node;
    auto transformation = Affine3f::Identity();
    while(!currentNode->isRootNode()) {
        transformation = currentNode->getTransform()->get() * transformation;
        currentNode = currentNode->getParent();
    }

    return Transform::create(transformation);
}

Eigen::Affine3f SceneGraph::getEigenTransformFromNode(
        SceneGraphNode::pointer node) {
    auto transformation = getTransformFromNode(node);

    Eigen::Affine3f result;
    result.matrix() = transformation->get().matrix();

    return result;
}
Eigen::Affine3f SceneGraph::getEigenTransformFromData(
        SpatialDataObject::pointer data) {
    SceneGraphNode::pointer currentNode = data->getSceneGraphNode();
    auto transformation = getTransformFromNode(currentNode);

    Eigen::Affine3f result;
    result.matrix() = transformation->get().matrix();

    return result;
}

Transform::pointer SceneGraph::getTransformFromData(
        SpatialDataObject::pointer data) {
    SceneGraphNode::pointer currentNode = data->getSceneGraphNode();
    return SceneGraph::getTransformFromNode(currentNode);
}

void SceneGraph::setParentNode(SpatialDataObject::pointer child,
        SpatialDataObject::pointer parent) {
    SceneGraphNode::pointer thisNode = child->getSceneGraphNode();
    SceneGraphNode::pointer parentNode = parent->getSceneGraphNode();
    thisNode->setParent(parentNode);
}


SceneGraphNode::pointer SceneGraph::insertParentNodeToData(SpatialDataObject::pointer child, Transform::pointer transform) {
    SceneGraphNode::pointer childNode = child->getSceneGraphNode();
    return insertParentNodeToNode(childNode, transform);
}

SceneGraphNode::pointer SceneGraph::insertParentNodeToNode(SceneGraphNode::pointer childNode, Transform::pointer transform) {
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setTransform(childNode->getTransform());
    newNode->setParent(childNode->getParent());
    childNode->setTransform(transform);
    childNode->setParent(newNode);
    return newNode;
}

} // end namespace fast
