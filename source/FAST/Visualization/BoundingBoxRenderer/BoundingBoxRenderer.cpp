#include "BoundingBoxRenderer.hpp"
#include "FAST/Data/BoundingBox.hpp"
#include "FAST/Data/SpatialDataObject.hpp"
#include <FAST/Visualization/View.hpp>

namespace fast {

BoundingBoxRenderer::BoundingBoxRenderer(float borderSize, std::map<uint, Color> labelColors) {
    m_2Donly = true;
    createInputPort<BoundingBoxSet>(0, false);

    createShaderProgram({
        Config::getKernelSourcePath() + "Visualization/BoundingBoxRenderer/BoundingBoxRenderer.vert",
        Config::getKernelSourcePath() + "Visualization/BoundingBoxRenderer/BoundingBoxRenderer.frag",
        Config::getKernelSourcePath() + "Visualization/BoundingBoxRenderer/BoundingBoxRenderer.geom",
    });

    setColors(labelColors);
    setBorderSize(borderSize);
}


BoundingBoxRenderer::~BoundingBoxRenderer() {
	glDeleteBuffers(1, &m_colorsUBO);
}

void BoundingBoxRenderer::setBorderSize(float borderSize) {
    m_borderSize = borderSize;
}

void BoundingBoxRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D,
                               int viewWidth,
                               int viewHeight) {
    if(!mode2D)
        throw Exception("BoundingBoxRenderer has only been implemented for 2D so far");

    auto dataToRender = getDataToRender();
    createColorUniformBufferObject();

	glDisable(GL_DEPTH_TEST);
    activateShader();
    setShaderUniform("perspectiveTransform", perspectiveMatrix);
    setShaderUniform("viewTransform", viewingMatrix);
    auto colorsIndex = glGetUniformBlockIndex(getShaderProgram(), "Colors");   
	glUniformBlockBinding(getShaderProgram(), colorsIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_colorsUBO); 
    // For all input data
    for(auto it : dataToRender) {
        auto boxes = std::static_pointer_cast<BoundingBoxSet>(it.second);
        float borderSize = m_borderSize;
        if(borderSize <= 0)
            borderSize = boxes->getMinimumSize()*0.1f;
        setShaderUniform("borderSize", borderSize);
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

        Affine3f transform = Affine3f::Identity();
        // If rendering is in 2D mode we skip any transformations
        if(!mode2D) {
            transform = SceneGraph::getEigenTransformFromData(it.second);
        }
        setShaderUniform("transform", transform);

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
		glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(uchar), nullptr);
        glEnableVertexAttribArray(1);

		GLuint EBO = access->getLinesEBO();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_LINES, boxes->getNrOfLines() * 2, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
    deactivateShader();
    glFinish(); // Fixes random crashes in OpenGL on NVIDIA windows due to some interaction with the text renderer. Suboptimal solution as glFinish is a blocking sync operation.
    glEnable(GL_DEPTH_TEST);
}

float BoundingBoxRenderer::getBorderSize() const {
    return m_borderSize;
}

std::string BoundingBoxRenderer::attributesToString() {
    std::stringstream ss;
    ss << "Attribute disabled " << (isDisabled() ? "true" : "false") << "\n";
    return ss.str();
}

void BoundingBoxRenderer::loadAttributes() {
    Renderer::loadAttributes();
}

}
