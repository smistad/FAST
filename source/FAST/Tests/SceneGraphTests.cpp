#include "catch.hpp"
#include "FAST/SceneGraph.hpp"
#include "DummyObjects.hpp"

namespace fast {

TEST_CASE("AffineTransformation object is initialized to 4x4 identity matrix", "[fast][SceneGraph]") {
    auto T = Transform::create();

    bool correctValues = true;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(i == j && T->get().matrix()(i,j) != 1) {
                correctValues = false;
            } else if(i != j && T->get().matrix()(i,j) != 0) {
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


    Affine3f asd = Affine3f::Identity();
    asd.matrix()(1,3) = 2.0;
    auto T = Transform::create(asd);
    DummyDataObject::pointer dummy = DummyDataObject::New();
    SceneGraphNode::pointer node = dummy->getSceneGraphNode();
    node->setTransform(T);

    auto T2 = SceneGraph::getTransformFromData(dummy);
    bool correctValues = true;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(T2->get().matrix()(i,j) != T->get().matrix()(i,j))
                correctValues = false;
        }
    }
    CHECK(correctValues == true);
}


} // end namespace fast
