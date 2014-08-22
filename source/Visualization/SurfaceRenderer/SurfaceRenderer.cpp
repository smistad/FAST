#include <GL/glew.h>
#include "SurfaceRenderer.hpp"
#include "Image.hpp"
#include "HelperFunctions.hpp"
#include "DeviceManager.hpp"
#include "View.hpp"
#include "Utility.hpp"
#include <QCursor>
#include "SceneGraph.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace fast {

void SurfaceRenderer::setInput(Surface::pointer image) {
    mInput = image;
    setParent(mInput);
    mIsModified = true;
}

SurfaceRenderer::SurfaceRenderer() : Renderer() {
    mDevice = DeviceManager::getInstance().getDefaultVisualizationDevice();
}

void SurfaceRenderer::execute() {
}

void SurfaceRenderer::draw() {
    // Draw the triangles in the VBO

    SceneGraph& graph = SceneGraph::getInstance();
    SceneGraphNode::pointer node = graph.getDataNode(mInput);
    LinearTransformation transform = graph.getLinearTransformationFromNode(node);

    float matrix[16] = {
            transform(0,0), transform(1,0), transform(2,0), transform(3,0),
            transform(0,1), transform(1,1), transform(2,1), transform(3,1),
            transform(0,2), transform(1,2), transform(2,2), transform(3,2),
            transform(0,3), transform(1,3), transform(2,3), transform(3,3)
    };

    glMultMatrixf(matrix);

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);

    // Set material properties which will be assigned by glColor
    GLfloat color[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
    GLfloat specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specReflection);
    GLfloat shininess[] = { 16.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    VertexBufferObjectAccess access = mInput->getVertexBufferObjectAccess(ACCESS_READ, mDevice);
    GLuint* VBO_ID = access.get();

    // Normal Buffer
    glBindBuffer(GL_ARRAY_BUFFER, *VBO_ID);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 24, BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, 24, BUFFER_OFFSET(12));

    glDrawArrays(GL_TRIANGLES, 0, mInput->getNrOfTriangles()*3);

    // Release buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glDisable(GL_LIGHTING);
}

BoundingBox SurfaceRenderer::getBoundingBox() {
    SceneGraph& graph = SceneGraph::getInstance();
    SceneGraphNode::pointer node = graph.getDataNode(mInput);
    LinearTransformation transform = graph.getLinearTransformationFromNode(node);
    BoundingBox inputBoundingBox = mInput->getBoundingBox();
    BoundingBox transformedBoundingBox = inputBoundingBox.getTransformedBoundingBox(transform);
    return transformedBoundingBox;
}

} // namespace fast
