#include "PointRenderer.hpp"
#include "SceneGraph.hpp"

namespace fast {

void PointRenderer::setInput(PointSet::pointer points) {
    setInputData(0, points);
}

void PointRenderer::draw() {
    PointSet::pointer points = getInputData(0);
    points->update();
    PointSetAccess access = points->getAccess(ACCESS_READ);
    MatrixXf pointMatrix = access.getPointSetAsMatrix();

    /*
    SceneGraph& graph = SceneGraph::getInstance();
    SceneGraphNode::pointer node = graph.getDataNode(points);
    LinearTransformation transform = graph.getLinearTransformationFromNode(node);

    glMultMatrixf(transform.getTransform().data());
    */

    glPointSize(mPointSize);
    glColor3f(1, 0, 0);
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_POINTS);
    for(uint i = 0; i < pointMatrix.cols(); i++) {
        glVertex3f(pointMatrix(0, i), pointMatrix(1, i), pointMatrix(2, i));
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
}

BoundingBox PointRenderer::getBoundingBox() {
    return BoundingBox(Float3(0,0,0));
}

PointRenderer::PointRenderer() {
    mPointSize = 10;
}

void PointRenderer::execute() {
}

}


