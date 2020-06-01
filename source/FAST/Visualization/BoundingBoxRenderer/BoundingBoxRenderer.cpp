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

    m_labelColors = {
        {0, Color::Green()},
        {1, Color::Blue()},
        {2, Color::Red()},
        {3, Color::Yellow()},
        {4, Color::Cyan()},
        {5, Color::Magenta()},
        {6, Color::Brown()},
        {255, Color::Cyan()},
    };
}

void BoundingBoxRenderer::setLabelColor(int label, Color color) {
    m_labelColors[label] = color;
    m_colorsModified = true;
}

BoundingBoxRenderer::~BoundingBoxRenderer() {
	glDeleteBuffers(1, &m_colorsUBO);
}

void BoundingBoxRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);

    if(!mode2D)
        throw Exception("BoundingBoxRenderer has only been implemented for 2D so far");

    if(m_colorsModified) {
        // Create UBO for colors
        glDeleteBuffers(1, &m_colorsUBO);
        glGenBuffers(1, &m_colorsUBO);
        int maxLabel = 0;
        for(auto&& labelColor : m_labelColors) {
            if(labelColor.first > maxLabel)
                maxLabel = labelColor.first;
        }
        auto colorData = std::make_unique<float[]>((maxLabel + 1) * 4);
        for(int i = 0; i <= maxLabel; ++i) {
            if(m_labelColors.count(i) > 0) {
                colorData[i * 4 + 0] = m_labelColors[i].getRedValue();
                colorData[i * 4 + 1] = m_labelColors[i].getGreenValue();
                colorData[i * 4 + 2] = m_labelColors[i].getBlueValue();
            } else {
                colorData[i * 4 + 0] = m_defaultColor.getRedValue();
                colorData[i * 4 + 1] = m_defaultColor.getGreenValue();
                colorData[i * 4 + 2] = m_defaultColor.getBlueValue();
            }
			colorData[i * 4 + 3] = 1.0f;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, m_colorsUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * (maxLabel+1), colorData.get(), GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        m_colorsModified = false;
    }

	glDisable(GL_DEPTH_TEST);
    activateShader();
    setShaderUniform("perspectiveTransform", perspectiveMatrix);
    setShaderUniform("viewTransform", viewingMatrix);
    auto colorsIndex = glGetUniformBlockIndex(getShaderProgram(), "Colors");   
	glUniformBlockBinding(getShaderProgram(), colorsIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_colorsUBO); 
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
        //setShaderUniform("globalColor", color.asVector());

        // Label data
        GLuint labelVBO = access->getLabelVBO();
		glBindBuffer(GL_ARRAY_BUFFER, labelVBO);
		glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(uchar), (void*)0);
        glEnableVertexAttribArray(1);

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
