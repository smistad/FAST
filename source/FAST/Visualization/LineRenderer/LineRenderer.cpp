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

    activateShader();
    setShaderUniform("perspectiveTransform", perspectiveMatrix);
    setShaderUniform("viewTransform", viewingMatrix);
    // For all input data
    for(auto it : mDataToRender) {
        Mesh::pointer points = it.second;

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(points);
        setShaderUniform("transform", transform->getTransform());

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

        VertexBufferObjectAccess::pointer access = points->getVertexBufferObjectAccess(ACCESS_READ);

        // Coordinates
        GLuint* coordinatesVBO = access->getCoordinateVBO();
        glBindBuffer(GL_ARRAY_BUFFER, *coordinatesVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        if(access->hasEBO()) {
            GLuint* EBO = access->getLineEBO();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
            glDrawElements(GL_LINES, points->getNrOfLines(), GL_UNSIGNED_INT, NULL);
        } else {
            // No EBO available; assume all vertices belong to lines consecutively
            glDrawArrays(GL_LINES, 0, points->getNrOfLines() * 2);
        }

        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
    }
    deactivateShader();
}

LineRenderer::LineRenderer() {
    createInputPort<Mesh>(0, false);
    mDefaultLineWidth = 2;
    mDefaultColor = Color::Blue();
    mDefaultDrawOnTop = false;
    createShaderProgram({
        Config::getKernelSourcePath() + "Visualization/LineRenderer/LineRenderer.vert",
        Config::getKernelSourcePath() + "Visualization/LineRenderer/LineRenderer.frag",
    });
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
