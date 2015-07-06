#include <GL/glew.h>
#include "HelperFunctions.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Visualization/View.hpp"
#include "FAST/Utility.hpp"
#include <QCursor>
#include <boost/thread/lock_guard.hpp>
#include "MeshRenderer.hpp"
#include "FAST/SceneGraph.hpp"

namespace fast {

void MeshRenderer::addInputConnection(ProcessObjectPort port) {
    uint nr = getNrOfInputData();
    if(nr > 0)
        createInputPort<Mesh>(nr);
    releaseInputAfterExecute(nr, false);
    setInputConnection(nr, port);
    mIsModified = true;
}

void MeshRenderer::addInputConnection(ProcessObjectPort port, Color color, float opacity) {
    addInputConnection(port);
    mInputColors[port] = color;
    mInputOpacities[port] = opacity;
}


MeshRenderer::MeshRenderer() : Renderer() {
    mDefaultOpacity = 1;
    mDefaultColor = Color::Green();
    mDefaultSpecularReflection = 0.8f;
    createInputPort<Mesh>(0, false);
}

void MeshRenderer::execute() {
    boost::lock_guard<boost::mutex> lock(mMutex);
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        Mesh::pointer input = getStaticInputData<Mesh>(inputNr);
        mMeshToRender[inputNr] = input;
    }
}

void MeshRenderer::draw() {
    boost::lock_guard<boost::mutex> lock(mMutex);

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);

    boost::unordered_map<uint, Mesh::pointer>::iterator it;
    for(it = mMeshToRender.begin(); it != mMeshToRender.end(); it++) {
        Mesh::pointer surfaceToRender = it->second;
        // Draw the triangles in the VBO
        AffineTransformation transform = SceneGraph::getAffineTransformationFromData(surfaceToRender);

        glPushMatrix();
        glMultMatrixf(transform.data());

        float opacity = mDefaultOpacity;
        Color color = mDefaultColor;
        ProcessObjectPort port = getInputPort(it->first);
        if(mInputOpacities.count(port) > 0) {
            opacity = mInputOpacities[port];
        }
        if(mInputColors.count(port) > 0) {
            color = mInputColors[port];
        }

        // Set material properties
        if(opacity < 1) {
            // Enable transparency
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        GLfloat GLcolor[] = { color.getRedValue(), color.getGreenValue(), color.getBlueValue(), opacity };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GLcolor);
        GLfloat specReflection[] = { mDefaultSpecularReflection, mDefaultSpecularReflection, mDefaultSpecularReflection, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specReflection);
        GLfloat shininess[] = { 16.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

        VertexBufferObjectAccess::pointer access = surfaceToRender->getVertexBufferObjectAccess(ACCESS_READ, getMainDevice());
        GLuint* VBO_ID = access->get();

        // Normal Buffer
        glBindBuffer(GL_ARRAY_BUFFER, *VBO_ID);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, 24, 0);
        glNormalPointer(GL_FLOAT, 24, (float*)(sizeof(GLfloat)*3));

        glDrawArrays(GL_TRIANGLES, 0, surfaceToRender->getNrOfTriangles()*3);

        // Release buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if(opacity < 1) {
            // Disable transparency
            glDisable(GL_BLEND);
        }
        glPopMatrix();
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
}

BoundingBox MeshRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    for(uint i = 0; i < getNrOfInputData(); i++) {
        BoundingBox transformedBoundingBox = mMeshToRender[i]->getTransformedBoundingBox();
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

void MeshRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
}

void MeshRenderer::setColor(ProcessObjectPort port, Color color) {
    mInputColors[port] = color;
}

void MeshRenderer::setOpacity(ProcessObjectPort port, float opacity) {
    mInputOpacities[port] = opacity;
}

void MeshRenderer::setDefaultOpacity(float opacity) {
    mDefaultOpacity = opacity;
    if(mDefaultOpacity > 1) {
        mDefaultOpacity = 1;
    } else if(mDefaultOpacity < 0) {
        mDefaultOpacity = 0;
    }
}

void MeshRenderer::setDefaultSpecularReflection(float specularReflection) {
    mDefaultSpecularReflection = specularReflection;
}

} // namespace fast
