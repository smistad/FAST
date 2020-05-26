#include "BoundingBoxRenderer.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include "FAST/Data/SpatialDataObject.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace fast {

BoundingBoxRenderer::BoundingBoxRenderer() {
    createInputPort<BoundingBoxSet>(0, false);

    createShaderProgram({
        Config::getKernelSourcePath() + "Visualization/BoundingBoxRenderer/BoundingBoxRenderer.vert",
        Config::getKernelSourcePath() + "Visualization/BoundingBoxRenderer/BoundingBoxRenderer.frag",
    });
}

void BoundingBoxRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);

    if(!mode2D)
        throw Exception("BoundingBoxRenderer has only been implemented for 2D so far");

	glDisable(GL_DEPTH_TEST);
    activateShader();
    setShaderUniform("perspectiveTransform", perspectiveMatrix);
    setShaderUniform("viewTransform", viewingMatrix);
    // For all input data
    for(auto it : mDataToRender) {
        auto boxes = std::static_pointer_cast<BoundingBoxSet>(it.second);
        if(boxes->getNrOfLines() == 0)
            continue;

        // TODO if VAO already exists, and data has not changed...

        // Delete old VAO
        if(mVAO.count(it.first) > 0) {
            glDeleteVertexArrays(1, &mVAO[it.first]);
        }
        // Create VAO
        uint VAO_ID;
        glGenVertexArrays(1, &VAO_ID);
        mVAO[it.first] = VAO_ID;
        glBindVertexArray(VAO_ID);

        AffineTransformation::pointer transform;
        if(mode2D) {
            // If rendering is in 2D mode we skip any transformations
            transform = AffineTransformation::New();
        } else {
            transform = SceneGraph::getAffineTransformationFromData(it.second);
        }
        setShaderUniform("transform", transform->getTransform());

        Color color = m_defaultColor;
        
        auto access = boxes->getOpenGLAccess(ACCESS_READ);

        // Coordinates
        GLuint coordinatesVBO = access->getCoordinateVBO();
        glBindBuffer(GL_ARRAY_BUFFER, coordinatesVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Color             
        bool useGlobalColor = true;
        //setShaderUniform("useGlobalColor", useGlobalColor);
        setShaderUniform("globalColor", color.asVector());

		GLuint EBO = access->getLinesEBO();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_LINES, boxes->getNrOfLines() * 2, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
    }
    deactivateShader();
    glFinish(); // Fixes random crashes in OpenGL on NVIDIA windows due to some interaction with the text renderer. Suboptimal solution as glFinish is a blocking sync operation.
    glEnable(GL_DEPTH_TEST);
}

}
