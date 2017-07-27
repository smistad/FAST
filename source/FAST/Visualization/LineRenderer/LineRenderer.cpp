#include "LineRenderer.hpp"
#include "FAST/Data/Access/MeshAccess.hpp"
#include "FAST/Data/Mesh.hpp"
#include "FAST/SceneGraph.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace fast {

void LineRenderer::draw() {
    std::lock_guard<std::mutex> lock(mMutex);

    // For all input data
    for(auto it : mDataToRender) {
        Mesh::pointer points = it.second;
        MeshAccess::pointer access = points->getMeshAccess(ACCESS_READ);

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(points);

        glPushMatrix();
        glMultMatrixf(transform->getTransform().data());

        DataPort::pointer port = getInputPort(it.first);

        if(mInputWidths.count(port) > 0) {
            glLineWidth(mInputWidths[port]);
        } else {
            glLineWidth(mDefaultLineWidth);
        }
        if(mInputColors.count(port) > 0) {
            Color c = mInputColors[port];
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        } else {
            Color c = mDefaultColor;
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        }
        bool drawOnTop;
        if(mInputDrawOnTop.count(port) > 0) {
            drawOnTop = mInputDrawOnTop[port];
        } else {
            drawOnTop = mDefaultDrawOnTop;
        }
        if(drawOnTop)
            glDisable(GL_DEPTH_TEST);
        glBegin(GL_LINES);
        for(MeshLine line : access->getLines()) {
            Vector3f a = access->getVertex(line.getEndpoint1()).getPosition();
            Vector3f b = access->getVertex(line.getEndpoint2()).getPosition();
            glVertex3f(a.x(), a.y(), a.z());
            glVertex3f(b.x(), b.y(), b.z());
        }
        glEnd();
        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
        glPopMatrix();
    }
    glColor3f(1.0f, 1.0f, 1.0f); // Reset color
}

LineRenderer::LineRenderer() {
    createInputPort<Mesh>(0, false);
    mDefaultLineWidth = 2;
    mDefaultColor = Color::Blue();
    mDefaultDrawOnTop = false;
}

void LineRenderer::addInputConnection(DataPort::pointer port) {
    Renderer::addInputConnection(port);
}

void LineRenderer::addInputConnection(DataPort::pointer port, Color color,
        float width) {
    addInputConnection(port);
    setColor(port, color);
    setWidth(port, width);
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

void LineRenderer::setDrawOnTop(DataPort::pointer port, bool drawOnTop) {
    mInputDrawOnTop[port] = drawOnTop;
}

void LineRenderer::setColor(DataPort::pointer port, Color color) {
    mInputColors[port] = color;
}

void LineRenderer::setWidth(DataPort::pointer port, float width) {
    mInputWidths[port] = width;
}

} // end namespace fast
