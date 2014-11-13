#include "catch.hpp"
#include "SceneGraph.hpp"
#include "DummyObjects.hpp"

namespace fast {

TEST_CASE("LinearTransformation object is initialized to 4x4 identity matrix", "[fast][SceneGraph]") {
    LinearTransformation T;

    bool correctValues = true;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(i == j && T(i,j) != 1) {
                correctValues = false;
            } else if(i != j && T(i,j) != 0) {
                correctValues = false;
            }
        }
    }
    CHECK(correctValues == true);
}

TEST_CASE("Inversion of linear transformation") {
    // TODO
}

TEST_CASE("Add data node to new root", "[fast][SceneGraph]") {
    SceneGraph& graph = SceneGraph::getInstance();

    DummyDataObject::pointer dummy = DummyDataObject::New();
    SceneGraphNode::pointer node = graph.addDataNodeToNewRoot(dummy);

    CHECK(node->getData().lock() == dummy);
    CHECK(node->isDataNode() == true);
    CHECK(node->isRootNode() == false);

    SceneGraphNode::pointer root = node->getParent();

    CHECK(root->isDataNode() == false);
    CHECK(root->isRootNode() == true);

    graph.deleteGraph();
}

TEST_CASE("Add data node with parent", "[fast][SceneGraph]") {
    SceneGraph& graph = SceneGraph::getInstance();

    DummyDataObject::pointer dummy1 = DummyDataObject::New();
    DummyDataObject::pointer dummy2 = DummyDataObject::New();
    SceneGraphNode::pointer node1 = graph.addDataNodeToNewRoot(dummy1);
    SceneGraphNode::pointer node2 = graph.addDataNode(dummy2, node1);

    CHECK(node2->getData().lock() == dummy2);
    CHECK(node2->isDataNode() == true);
    CHECK(node2->isRootNode() == false);
    CHECK(node2->getParent() == node1);

    graph.deleteGraph();
}

TEST_CASE("Get linear transformation from node to its parent root", "[fast][SceneGraph]") {
    SceneGraph& graph = SceneGraph::getInstance();

    LinearTransformation T;
    T(1,3) = 2.0;
    DummyDataObject::pointer dummy = DummyDataObject::New();
    SceneGraphNode::pointer node = graph.addDataNodeToNewRoot(dummy);
    node->setTransformation(T);

    LinearTransformation T2 = graph.getLinearTransformationFromNode(node);
    bool correctValues = true;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(T2(i,j) != T(i,j))
                correctValues = false;
        }
    }
    CHECK(correctValues == true);

    graph.deleteGraph();
}


} // end namespace fast
