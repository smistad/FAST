#include "PointRenderer.hpp"
#include "SceneGraph.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace fast {

void PointRenderer::draw() {
    // For all input data
    for(uint i = 0; i < getNrOfInputData(); i++) {
        PointSet::pointer points = getInputData(i);
        PointSetAccess access = points->getAccess(ACCESS_READ);
        MatrixXf pointMatrix = access.getPointSetAsMatrix();

        LinearTransformation transform = SceneGraph::getLinearTransformationFromData(points);

        glPushMatrix();
        glMultMatrixf(transform.getTransform().data());

        if(mInputSizes.count(points) > 0) {
            glPointSize(mInputSizes[points]);
        } else {
            glPointSize(mDefaultPointSize);
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
        glBegin(GL_POINTS);
        for(uint i = 0; i < pointMatrix.cols(); i++) {
            glVertex3f(pointMatrix(0, i), pointMatrix(1, i), pointMatrix(2, i));
        }
        glEnd();
        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
        glPopMatrix();
    }
}

BoundingBox PointRenderer::getBoundingBox() {
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

PointRenderer::PointRenderer() {
    mDefaultPointSize = 10;
    mDefaultColor = Color::Red();
    mDefaultDrawOnTop = false;
}

void PointRenderer::execute() {
}


void PointRenderer::addInput(PointSet::pointer points) {
    releaseInputAfterExecute(getNrOfInputData(), false);
    setInputData(getNrOfInputData(), points);
}

void PointRenderer::addInput(PointSet::pointer points, Color color,
        float size) {
    addInput(points);
    setColor(points, color);
    setSize(points, size);
}

void PointRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
}

void PointRenderer::setDefaultSize(float size) {
    mDefaultPointSize = size;
}

void PointRenderer::setDefaultDrawOnTop(bool drawOnTop) {
    mDefaultDrawOnTop = drawOnTop;
}


void PointRenderer::setDrawOnTop(DataObject::pointer input, bool drawOnTop) {
    mInputDrawOnTop[input] = drawOnTop;
}

void PointRenderer::setColor(DataObject::pointer input, Color color) {
    mInputColors[input] = color;
}

void PointRenderer::setSize(DataObject::pointer input, float size) {
    mInputSizes[input] = size;
}

} // end namespace fast
