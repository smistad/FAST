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
TEST_CASE("Get linear transformation from node to its parent root", "[fast][SceneGraph]") {

    LinearTransformation T;
    T(1,3) = 2.0;
    DummyDataObject::pointer dummy = DummyDataObject::New();
    SceneGraphNode::pointer node = dummy->getSceneGraphNode();
    node->setTransformation(T);

    LinearTransformation T2 = SceneGraph::getLinearTransformationFromData(dummy);
    bool correctValues = true;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(T2(i,j) != T(i,j))
                correctValues = false;
        }
    }
    CHECK(correctValues == true);
}


} // end namespace fast
