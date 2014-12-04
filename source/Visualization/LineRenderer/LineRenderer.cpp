#include "LineRenderer.hpp"
#include "LineSetAccess.hpp"
#include "SceneGraph.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace fast {

void LineRenderer::draw() {
    // For all input data
    for(uint i = 0; i < getNrOfInputData(); i++) {
        LineSet::pointer points = getInputData(i);
        LineSetAccess access = points->getAccess(ACCESS_READ);

        SceneGraph& graph = SceneGraph::getInstance();
        SceneGraphNode::pointer node = graph.getDataNode(points);
        LinearTransformation transform = graph.getLinearTransformationFromNode(node);

        glPushMatrix();
        glMultMatrixf(transform.getTransform().data());

        if(mInputWidths.count(points) > 0) {
            glLineWidth(mInputWidths[points]);
        } else {
            glLineWidth(mDefaultLineWidth);
        }
        if(mInputColors.count(points) > 0) {
            Color c = mInputColors[points];
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        } else {
            Color c = mDefaultColor;
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        }
        bool drawOnTop;
        if(mInputDrawOnTop.count(points) > 0) {
            drawOnTop = mInputDrawOnTop[points];
        } else {
            drawOnTop = mDefaultDrawOnTop;
        }
        if(drawOnTop)
            glDisable(GL_DEPTH_TEST);
        glBegin(GL_LINES);
        for(uint i = 0; i < access.getNrOfLines(); i++) {
            Vector2ui line = access.getLine(i);
            Vector3f a = access.getPoint(line.x());
            Vector3f b = access.getPoint(line.y());
            glVertex3f(a.x(), a.y(), a.z());
            glVertex3f(b.x(), b.y(), b.z());
        }
        glEnd();
        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
        glPopMatrix();
    }
}

BoundingBox LineRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    for(uint i = 0; i < getNrOfInputData(); i++) {
        BoundingBox transformedBoundingBox = getInputData(i)->getTransformedBoundingBox();
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

LineRenderer::LineRenderer() {
    mDefaultLineWidth = 2;
    mDefaultColor = Color::Blue();
    mDefaultDrawOnTop = false;
}

void LineRenderer::execute() {
}


void LineRenderer::addInput(LineSet::pointer lines) {
    releaseInputAfterExecute(getNrOfInputData(), false);
    setInputData(getNrOfInputData(), lines);
}

void LineRenderer::addInput(LineSet::pointer lines, Color color,
        float width) {
    addInput(lines);
    setColor(lines, color);
    setWidth(lines, width);
}

void LineRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
}

void LineRenderer::setDefaultLineWidth(float width) {
    mDefaultLineWidth = width;
}

void LineRenderer::setDefaultDrawOnTop(bool drawOnTop) {
    mDefaultDrawOnTop = drawOnTop;
}


void LineRenderer::setDrawOnTop(DataObject::pointer input, bool drawOnTop) {
    mInputDrawOnTop[input] = drawOnTop;
}

void LineRenderer::setColor(DataObject::pointer input, Color color) {
    mInputColors[input] = color;
}

void LineRenderer::setWidth(DataObject::pointer input, float width) {
    mInputWidths[input] = width;
}

} // end namespace fast
