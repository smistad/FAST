#include "VertexRenderer.hpp"
#include "FAST/SceneGraph.hpp"


namespace fast {

void VertexRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D,
                          int viewWidth,
                          int viewHeight) {
    glEnable(GL_POINT_SPRITE); // Circles created in fragment shader will not work without this
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    activateShader();
    setShaderUniform("perspectiveTransform", perspectiveMatrix);
    setShaderUniform("viewTransform", viewingMatrix);

    auto dataToRender = getDataToRender();
    for(auto it : dataToRender) {
        // Delete old VAO
        if(mVAO.count(it.first) > 0)
            glDeleteVertexArrays(1, &mVAO[it.first]);
        // Create VAO
        uint VAO_ID;
        glGenVertexArrays(1, &VAO_ID);
        mVAO[it.first] = VAO_ID;
        glBindVertexArray(VAO_ID);

        Mesh::pointer points = std::static_pointer_cast<Mesh>(it.second);
        float pointSize = mDefaultPointSize;
        if(mInputSizes.count(it.first) > 0) {
            pointSize = mInputSizes[it.first];
        }
        auto access = points->getVertexBufferObjectAccess(ACCESS_READ);
        bool useGlobalColor = false;
        Color color = Color::Green();
        if(mInputColors.count(it.first) > 0) {
            color = mInputColors[it.first];
            useGlobalColor = true;
        } else if(!mDefaultColor.isNull()) {
            color = mDefaultColor;
            useGlobalColor = true;
        } else if(!access->hasColorVBO()) {
            color = Color::Green();
            useGlobalColor = true;
        }
        bool drawOnTop;
        if(mInputDrawOnTop.count(it.first) > 0) {
            drawOnTop = mInputDrawOnTop[it.first];
        } else {
            drawOnTop = mDefaultDrawOnTop;
        }
        if(drawOnTop)
            glDisable(GL_DEPTH_TEST);

        auto transform = SceneGraph::getEigenTransformFromData(points);
        setShaderUniform("transform", transform);
        setShaderUniform("pointSize", pointSize);

        GLuint* coordinateVBO = access->getCoordinateVBO();

        // Coordinates buffer
        glBindBuffer(GL_ARRAY_BUFFER, *coordinateVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Color buffer
        if(access->hasColorVBO() && !useGlobalColor) {
            GLuint *colorVBO = access->getColorVBO();
            glBindBuffer(GL_ARRAY_BUFFER, *colorVBO);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
        } else {
            useGlobalColor = true;
        }
        setShaderUniform("useGlobalColor", useGlobalColor);
        setShaderUniform("globalColor", color.asVector());

        glDrawArrays(GL_POINTS, 0, points->getNrOfVertices());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if(drawOnTop)
            glEnable(GL_DEPTH_TEST);
    }

    deactivateShader();
}

VertexRenderer::VertexRenderer(float size, Color color, bool drawOnTop) {
    setDefaultSize(size);
    setDefaultColor(color);
    setDefaultDrawOnTop(drawOnTop);
    createInputPort(0, "Mesh");

    createShaderProgram({
        Config::getKernelSourcePath() + "Visualization/VertexRenderer/VertexRenderer.vert",
        Config::getKernelSourcePath() + "Visualization/VertexRenderer/VertexRenderer.frag",
    });
}


uint VertexRenderer::addInputConnection(DataChannel::pointer port) {
    return Renderer::addInputConnection(port);
}

uint VertexRenderer::addInputConnection(DataChannel::pointer port, Color color,
        float size) {
    uint nr = addInputConnection(port);
    setColor(nr, color);
    setSize(nr, size);
    return nr;
}

uint VertexRenderer::addInputData(DataObject::pointer data) {
    return Renderer::addInputData(data);
}

uint VertexRenderer::addInputData(Mesh::pointer data, Color color, float size) {
    uint nr = addInputData(data);
    setColor(nr, color);
    setSize(nr, size);
	return nr;
}


void VertexRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
}

void VertexRenderer::setDefaultSize(float size) {
    mDefaultPointSize = size;
}

void VertexRenderer::setDefaultDrawOnTop(bool drawOnTop) {
    mDefaultDrawOnTop = drawOnTop;
}


void VertexRenderer::setDrawOnTop(uint inputNr, bool drawOnTop) {
    mInputDrawOnTop[inputNr] = drawOnTop;
}

void VertexRenderer::setColor(uint inputNr, Color color) {
    mInputColors[inputNr] = color;
}

void VertexRenderer::setSize(uint inputNr, float size) {
    mInputSizes[inputNr] = size;
}

} // end namespace fast
