#include "FAST/Utility.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Visualization/View.hpp"
#include "FAST/Utility.hpp"
#include "FAST/Visualization/Window.hpp"
#include "TriangleRenderer.hpp"
#include "FAST/SceneGraph.hpp"

namespace fast {

uint TriangleRenderer::addInputConnection(DataPort::pointer port, Color color, float opacity) {
    uint nr = Renderer::addInputConnection(port);
    mInputColors[nr] = color;
    mInputOpacities[nr] = opacity;
    return nr;
}


TriangleRenderer::TriangleRenderer() : Renderer() {
    mDefaultOpacity = 1;
    mDefaultColor = Color::Green();
    mDefaultSpecularReflection = 0.8f;
    createInputPort<Mesh>(0, false);
    mLineSize = 1;
    mWireframe = false;
    mDefaultColorSet = false;
    createShaderProgram({
        Config::getKernelSourcePath() + "Visualization/TriangleRenderer/TriangleRenderer.vert",
        Config::getKernelSourcePath() + "Visualization/TriangleRenderer/TriangleRenderer.frag",
    });
}

void TriangleRenderer::setWireframe(bool wireframe) {
    mWireframe = wireframe;
}

void TriangleRenderer::setLineSize(int size) {
	if(size <= 0)
		throw Exception("Line size must be greather than 0");

	mLineSize = size;

}

void TriangleRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);

    if(mWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    activateShader();
    setShaderUniform("perspectiveTransform", perspectiveMatrix);
    setShaderUniform("viewTransform", viewingMatrix);
    setShaderUniform("mode2D", mode2D);
    setShaderUniform("ignoreInvertedNormals", mIgnoreInvertedNormals);
    for(auto it : mDataToRender) {
        Mesh::pointer surfaceToRender = std::static_pointer_cast<Mesh>(it.second);

        // Create VAO
        uint VAO_ID;
        glGenVertexArrays(1, &VAO_ID);
        glBindVertexArray(VAO_ID);

        AffineTransformation::pointer transform;
        if(mode2D) {
            // If rendering is in 2D mode we skip any transformations
            transform = AffineTransformation::New();
        } else {
            transform = SceneGraph::getAffineTransformationFromData(surfaceToRender);
        }

        setShaderUniform("transform", transform->getTransform());

        float opacity = mDefaultOpacity;
        if(mInputOpacities.count(it.first) > 0) {
            opacity = mInputOpacities[it.first];
        }

        Color color = mDefaultColor;
        bool useGlobalColor = false;
        if(mInputColors.count(it.first) > 0) {
            color = mInputColors[it.first];
            useGlobalColor = true;
        } else if(mDefaultColorSet) {
            color = mDefaultColor;
            useGlobalColor = true;
        }

        // Set material properties
        if(opacity < 1) {
            // Enable transparency
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        VertexBufferObjectAccess::pointer access = surfaceToRender->getVertexBufferObjectAccess(ACCESS_READ);

        // Coordinates
        GLuint* coordinatesVBO = access->getCoordinateVBO();
        glBindBuffer(GL_ARRAY_BUFFER, *coordinatesVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normals
        if(access->hasNormalVBO()) {
            GLuint* normalVBO = access->getNormalVBO();
            glBindBuffer(GL_ARRAY_BUFFER, *normalVBO);
            setShaderUniform("use_normals", true);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
        } else {
            setShaderUniform("use_normals", false);
        }

        // Color buffer
        if(access->hasColorVBO() && !useGlobalColor) {
            GLuint *colorVBO = access->getColorVBO();
            glBindBuffer(GL_ARRAY_BUFFER, *colorVBO);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(2);
        } else {
            useGlobalColor = true;
        }
        setShaderUniform("useGlobalColor", useGlobalColor);
        setShaderUniform("globalColor", color.asVector());
        setShaderUniform("opacity", opacity);

        if(access->hasEBO()) {
            GLuint* EBO = access->getTriangleEBO();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
            glDrawElements(GL_TRIANGLES, surfaceToRender->getNrOfTriangles() * 3, GL_UNSIGNED_INT, NULL);
        } else {
            // No EBO available; assume all vertices belong to triangles
            glDrawArrays(GL_TRIANGLES, 0, surfaceToRender->getNrOfTriangles() * 3);
        }

        // Release buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        if(opacity < 1) {
            // Disable transparency
            glDisable(GL_BLEND);
        }
    }

    if(mWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    deactivateShader();
}

void TriangleRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
    mDefaultColorSet = true;
}

void TriangleRenderer::setLabelColor(int label, Color color) {
	mLabelColors[label] = color;
}

void TriangleRenderer::setColor(uint inputNr, Color color) {
    mInputColors[inputNr] = color;
}

void TriangleRenderer::setOpacity(uint inputNr, float opacity) {
    mInputOpacities[inputNr] = opacity;
}

void TriangleRenderer::setDefaultOpacity(float opacity) {
    mDefaultOpacity = opacity;
    if(mDefaultOpacity > 1) {
        mDefaultOpacity = 1;
    } else if(mDefaultOpacity < 0) {
        mDefaultOpacity = 0;
    }
}

void TriangleRenderer::setDefaultSpecularReflection(float specularReflection) {
    mDefaultSpecularReflection = specularReflection;
}

void TriangleRenderer::setIgnoreInvertedNormals(bool ignore) {
    mIgnoreInvertedNormals = ignore;
}

uint TriangleRenderer::addInputConnection(DataPort::pointer port) {
    return Renderer::addInputConnection(port);
}

} // namespace fast
