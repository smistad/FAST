#include "FAST/SceneGraph.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

namespace fast {

void SceneGraphNode::reset() {
    // Remove previous parent
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    setParent(newRootNode);
    // Set transformation to identity
    setTransformation(AffineTransformation::New());
}

// Set transformation to its parent
void SceneGraphNode::setTransformation(
        AffineTransformation::pointer transformation) {
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

AffineTransformation::pointer SceneGraphNode::getTransformation() const {
    return mTransformation;
}

SceneGraphNode::SceneGraphNode() {
    mIsRootNode = true;
    mTransformation = AffineTransformation::New();
}

AffineTransformation::pointer SceneGraph::getAffineTransformationBetweenNodes(
        SceneGraphNode::pointer nodeA, SceneGraphNode::pointer nodeB) {
    // TODO traverse the graph from node A to node B

			return AffineTransformation::New();
}

AffineTransformation::pointer SceneGraph::getAffineTransformationFromNode(
        SceneGraphNode::pointer node) {
    SceneGraphNode::pointer currentNode = node;
    AffineTransformation::pointer transformation = AffineTransformation::New();
    while(!currentNode->isRootNode()) {
        transformation = currentNode->getTransformation()->multiply(transformation);
        currentNode = currentNode->getParent();
    }

    return transformation;
}

Eigen::Affine3f SceneGraph::getEigenAffineTransformationFromData(
        SpatialDataObject::pointer data) {
    SceneGraphNode::pointer currentNode = data->getSceneGraphNode();
    AffineTransformation::pointer transformation = getAffineTransformationFromNode(currentNode);

    Eigen::Affine3f result;
    result.matrix() = transformation->getTransform().matrix();

    return result;
}

AffineTransformation::pointer SceneGraph::getAffineTransformationFromData(
        SpatialDataObject::pointer data) {
    SceneGraphNode::pointer currentNode = data->getSceneGraphNode();
    return SceneGraph::getAffineTransformationFromNode(currentNode);
}

void SceneGraph::setParentNode(SpatialDataObject::pointer child,
        SpatialDataObject::pointer parent) {
    SceneGraphNode::pointer thisNode = child->getSceneGraphNode();
    SceneGraphNode::pointer parentNode = parent->getSceneGraphNode();
    thisNode->setParent(parentNode);
}


SceneGraphNode::pointer SceneGraph::insertParentNodeToData(SpatialDataObject::pointer child, AffineTransformation::pointer transform) {
    SceneGraphNode::pointer childNode = child->getSceneGraphNode();
    return insertParentNodeToNode(childNode, transform);
}

SceneGraphNode::pointer SceneGraph::insertParentNodeToNode(SceneGraphNode::pointer childNode, AffineTransformation::pointer transform) {
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setTransformation(childNode->getTransformation());
    newNode->setParent(childNode->getParent());
    childNode->setTransformation(transform);
    childNode->setParent(newNode);
    return newNode;
}

} // end namespace fast
