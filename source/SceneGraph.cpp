#include "SceneGraph.hpp"
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace boost::numeric::ublas;

namespace fast {

/**
 * Initializes linear transformation object to identity matrix
 */
LinearTransformation::LinearTransformation() : boost::numeric::ublas::matrix<float>(4,4) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            this->operator()(i,j) = i == j ? 1 : 0;
        }
    }
}

LinearTransformation LinearTransformation::getInverse() {
    typedef permutation_matrix<std::size_t> pmatrix;

    // create a working copy
    matrix<float> A = getMatrix();

    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());

    // perform LU-factorization
    int res = lu_factorize(A, pm);
    if (res != 0)
        throw Exception("Unable to invert matrix");

    // create identity matrix of "inverse"
    LinearTransformation inverse;
    inverse.assign(identity_matrix<float> (A.size1()));

    // backsubstitute to get the inverse
    lu_substitute(A, pm, inverse);

    return inverse;
}

void SceneGraphNode::setDataObject(DataObject::pointer data) {
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

DataObject::pointer SceneGraphNode::getData() const {
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

SceneGraphNode::pointer SceneGraph::addDataNode(DataObject::pointer data,
        SceneGraphNode::pointer parent) {
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setDataObject(data);
    newNode->setParent(parent);
    mNodes.insert(newNode);
    mDataToNodesMap[data] = newNode;
    return newNode;
}

SceneGraphNode::pointer SceneGraph::addDataNodeToNewRoot(
        DataObject::pointer data) {
    SceneGraphNode::pointer newRootNode = SceneGraphNode::New();
    SceneGraphNode::pointer newNode = SceneGraphNode::New();
    newNode->setDataObject(data);
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
        DataObject::pointer data) {
    return mDataToNodesMap[data];
}

void SceneGraph::removeDataNode(DataObject::pointer data) {
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

LinearTransformation LinearTransformation::operator *(
        const LinearTransformation& other) {
    LinearTransformation T(boost::numeric::ublas::prod((boost::numeric::ublas::matrix<float>)*this, (boost::numeric::ublas::matrix<float>)other));
    return T;
}

boost::numeric::ublas::matrix<float> LinearTransformation::getMatrix() const {
    return matrix<float>(*this);
}

} // end namespace fast
