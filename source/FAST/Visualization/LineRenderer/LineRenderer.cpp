#include "LineRenderer.hpp"
#include "FAST/Data/Access/LineSetAccess.hpp"
#include "FAST/Data/LineSet.hpp"
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
    std::unordered_map<uint, LineSet::pointer>::iterator it;
    for(it = mLineSetsToRender.begin(); it != mLineSetsToRender.end(); it++) {
        LineSet::pointer points = it->second;
        LineSetAccess::pointer access = points->getAccess(ACCESS_READ);

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(points);

        glPushMatrix();
        glMultMatrixf(transform->data());

        ProcessObjectPort port = getInputPort(it->first);

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
        for(uint i = 0; i < access->getNrOfLines(); i++) {
            Vector2ui line = access->getLine(i);
            Vector3f a = access->getPoint(line.x());
            Vector3f b = access->getPoint(line.y());
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

BoundingBox LineRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    for(uint i = 0; i < getNrOfInputData(); i++) {
        BoundingBox transformedBoundingBox = getStaticInputData<LineSet>(i)->getTransformedBoundingBox();
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

LineRenderer::LineRenderer() {
    createInputPort<LineSet>(0, false);
    mDefaultLineWidth = 2;
    mDefaultColor = Color::Blue();
    mDefaultDrawOnTop = false;
}

void LineRenderer::execute() {
    std::lock_guard<std::mutex> lock(mMutex);

    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        LineSet::pointer input = getStaticInputData<LineSet>(inputNr);

        mLineSetsToRender[inputNr] = input;
    }
}


void LineRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<LineSet>(nr);
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
}

void LineRenderer::addInputConnection(ProcessObjectPort port, Color color,
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


void LineRenderer::setDrawOnTop(ProcessObjectPort port, bool drawOnTop) {
    mInputDrawOnTop[port] = drawOnTop;
}

void LineRenderer::setColor(ProcessObjectPort port, Color color) {
    mInputColors[port] = color;
}

void LineRenderer::setWidth(ProcessObjectPort port, float width) {
    mInputWidths[port] = width;
}

} // end namespace fast
