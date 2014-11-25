#include <GL/glew.h>
#include "Image.hpp"
#include "HelperFunctions.hpp"
#include "DeviceManager.hpp"
#include "View.hpp"
#include "Utility.hpp"
#include <QCursor>

#include "MeshRenderer.hpp"
#include "SceneGraph.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace fast {

void SurfaceRenderer::setInput(MeshData::pointer image) {
    mInput = image;
    setParent(mInput);
    mIsModified = true;
}

SurfaceRenderer::SurfaceRenderer() : Renderer() {
    mDevice = DeviceManager::getInstance().getDefaultVisualizationDevice();
    mOpacity = 1;
}

void SurfaceRenderer::execute() {
    if(mInput->isDynamicData()) {
        mSurfaceToRender = DynamicMesh::pointer(mInput)->getNextFrame();
    } else {
        mSurfaceToRender = mInput;
    }
}

void SurfaceRenderer::draw() {
    // Draw the triangles in the VBO

    SceneGraph& graph = SceneGraph::getInstance();
    SceneGraphNode::pointer node = graph.getDataNode(mSurfaceToRender);
    LinearTransformation transform = graph.getLinearTransformationFromNode(node);

    glMultMatrixf(transform.getTransform().data());

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);

    // Set material properties which will be assigned by glColor
    if(mOpacity < 1) {
        // Enable transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    GLfloat color[] = { 0.0f, 1.0f, 0.0f, mOpacity };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
    GLfloat specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specReflection);
    GLfloat shininess[] = { 16.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    releaseOpenGLContext();
    VertexBufferObjectAccess access = mSurfaceToRender->getVertexBufferObjectAccess(ACCESS_READ, mDevice);
    setOpenGLContext(mDevice->getGLContext());
    GLuint* VBO_ID = access.get();

    // Normal Buffer
    glBindBuffer(GL_ARRAY_BUFFER, *VBO_ID);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 24, BUFFER_OFFSET(0));
    glNormalPointer(GL_FLOAT, 24, BUFFER_OFFSET(12));

    glDrawArrays(GL_TRIANGLES, 0, mSurfaceToRender->getNrOfTriangles()*3);

    // Release buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glDisable(GL_LIGHTING);
}

BoundingBox SurfaceRenderer::getBoundingBox() {
    SceneGraph& graph = SceneGraph::getInstance();
    SceneGraphNode::pointer node = graph.getDataNode(mSurfaceToRender);
    LinearTransformation transform = graph.getLinearTransformationFromNode(node);
    BoundingBox inputBoundingBox = mSurfaceToRender->getBoundingBox();
    BoundingBox transformedBoundingBox = inputBoundingBox.getTransformedBoundingBox(transform);
    return transformedBoundingBox;
}

void SurfaceRenderer::setOpacity(float opacity) {
    mOpacity = opacity;
    if(mOpacity > 1) {
        mOpacity = 1;
    } else if(mOpacity < 0) {
        mOpacity = 0;
    }
}

} // namespace fast
