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

void LineRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix) {
    std::lock_guard<std::mutex> lock(mMutex);

    // For all input data
    for(auto it : mDataToRender) {
        Mesh::pointer points = it.second;
        MeshAccess::pointer access = points->getMeshAccess(ACCESS_READ);

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(points);

        glPushMatrix();
        glMultMatrixf(transform->getTransform().data());

        if(mInputWidths.count(it.first) > 0) {
            glLineWidth(mInputWidths[it.first]);
        } else {
            glLineWidth(mDefaultLineWidth);
        }
        if(mInputColors.count(it.first) > 0) {
            Color c = mInputColors[it.first];
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        } else {
            Color c = mDefaultColor;
            glColor3f(c.getRedValue(), c.getGreenValue(), c.getBlueValue());
        }
        bool drawOnTop;
        if(mInputDrawOnTop.count(it.first) > 0) {
            drawOnTop = mInputDrawOnTop[it.first];
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

uint LineRenderer::addInputConnection(DataPort::pointer port) {
    return Renderer::addInputConnection(port);
}

uint LineRenderer::addInputConnection(DataPort::pointer port, Color color,
        float width) {
    uint nr = addInputConnection(port);
    setColor(nr, color);
    setWidth(nr, width);
    return nr;
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

void LineRenderer::setDrawOnTop(uint inputNr, bool drawOnTop) {
    mInputDrawOnTop[inputNr] = drawOnTop;
}

void LineRenderer::setColor(uint nr, Color color) {
    mInputColors[nr] = color;
}

void LineRenderer::setWidth(uint nr, float width) {
    mInputWidths[nr] = width;
}

} // end namespace fast
