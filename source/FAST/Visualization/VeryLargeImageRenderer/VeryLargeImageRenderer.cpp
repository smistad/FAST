#include "VeryLargeImageRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"
#include <FAST/Data/WholeSlideImage.hpp>
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#include <CL/cl_gl.h>
#else
#include <CL/cl_gl.h>

#endif
#endif


namespace fast {




VeryLargeImageRenderer::VeryLargeImageRenderer() : Renderer() {
    createInputPort<WholeSlideImage>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.cl", "3D");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer2D.cl", "2D");
    mIsModified = true;
    mWindow = -1;
    mLevel = -1;
    createFloatAttribute("window", "Intensity window", "Intensity window", -1);
    createFloatAttribute("level", "Intensity level", "Intensity level", -1);
    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/VeryLargeImageRenderer/VeryLargeImageRenderer.frag",
                        });
}

void VeryLargeImageRenderer::setIntensityLevel(float level) {
    mLevel = level;
}

float VeryLargeImageRenderer::getIntensityLevel() {
    return mLevel;
}

void VeryLargeImageRenderer::setIntensityWindow(float window) {
    if (window <= 0)
        throw Exception("Intensity window has to be above 0.");
    mWindow = window;
}

float VeryLargeImageRenderer::getIntensityWindow() {
    return mWindow;
}

void VeryLargeImageRenderer::loadAttributes() {
    mWindow = getFloatAttribute("window");
    mLevel = (getFloatAttribute("level"));
}

void VeryLargeImageRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);

    Vector4f bottom_left = (perspectiveMatrix*viewingMatrix).inverse()*Vector4f(-1,-1,0,1);
    Vector4f top_right = (perspectiveMatrix*viewingMatrix).inverse()*Vector4f(1,1,0,1);
    float width = top_right.x() - bottom_left.x();
    float height = top_right.y() - bottom_left.y();
    std::cout << "Viewing coordinates:" << bottom_left.transpose() << " " <<  top_right.transpose() << std::endl;
    std::cout << "Current Size:" << width << " " <<  height << std::endl;

    for(auto it : mDataToRender) {
        auto input = std::static_pointer_cast<WholeSlideImage>(it.second);
        int fullWidth = input->m_levels.back().width;
        int fullHeight = input->m_levels.back().height;
        std::cout << "scaling: " << fullWidth/width << std::endl;
        int levelToUse = round(std::log(fullWidth/width));
        if(levelToUse >= input->m_levels.size())
            levelToUse = input->m_levels.size()-1;
        std::cout << "Level to use: " << levelToUse << std::endl;
        uint inputNr = it.first;

        // Check if a texture has already been created for this image
        if(mTexturesToRender.count(inputNr) > 0 && mImageUsed[inputNr] == input && mCurrentLevel == levelToUse)
            continue; // If it has already been created, skip it

        // If it has not been created, create the texture
        mCurrentLevel = levelToUse;

        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
            glDeleteVertexArrays(1, &mVAO[inputNr]);
            mVAO.erase(inputNr);
        }

        GLuint textureID;

        auto level = input->m_levels[levelToUse];
        auto data = level.data;
        // Copy data from CPU to GL texture
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, level.width, level.height, 0, GL_RGBA,  GL_UNSIGNED_BYTE, data.get());
        glBindTexture(GL_TEXTURE_2D, 0);
        glFinish();

        mTexturesToRender[inputNr] = textureID;
        mImageUsed[inputNr] = input;
    }

    drawTextures(perspectiveMatrix, viewingMatrix, mode2D);

}

void VeryLargeImageRenderer::drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D) {

    for(auto it : mDataToRender) {
        auto input = std::static_pointer_cast<WholeSlideImage>(it.second);
        uint inputNr = it.first;
        // Delete old VAO
        if(mVAO.count(inputNr) > 0)
            glDeleteVertexArrays(1, &mVAO[inputNr]);
        // Create VAO
        uint VAO_ID;
        glGenVertexArrays(1, &VAO_ID);
        mVAO[inputNr] = VAO_ID;
        glBindVertexArray(VAO_ID);

        // Create VBO
        // Get width and height in mm
        auto level = input->m_levels.back();
        float width = level.width;// * input->getSpacing().x();
        float height = level.height;// * input->getSpacing().y();
        float vertices[] = {
                // vertex: x, y, z; tex coordinates: x, y
                0.0f, height, 0.0f, 0.0f, 0.0f,
                width, height, 0.0f, 1.0f, 0.0f,
                width, 0.0f, 0.0f, 1.0f, 1.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        };
        // Delete old VBO
        if(mVBO.count(inputNr) > 0)
            glDeleteBuffers(1, &mVBO[inputNr]);
        uint VBO;
        glGenBuffers(1, &VBO);
        mVBO[inputNr] = VBO;
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // Delete old EBO
        if(mEBO.count(inputNr) > 0)
            glDeleteBuffers(1, &mEBO[inputNr]);
        // Create EBO
        uint EBO;
        glGenBuffers(1, &EBO);
        mEBO[inputNr] = EBO;
        uint indices[] = {  // note that we start from 0!
                0, 1, 3,   // first triangle
                1, 2, 3    // second triangle
        };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);

    }

    activateShader();

    // This is the actual rendering
    for(auto it : mImageUsed) {
        AffineTransformation::pointer transform;
        if(mode2D) {
            // If rendering is in 2D mode we skip any transformations
            transform = AffineTransformation::New();
        } else {
            transform = SceneGraph::getAffineTransformationFromData(it.second);
        }

        //transform->getTransform().scale(it.second->getSpacing());

        uint transformLoc = glGetUniformLocation(getShaderProgram(), "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform->getTransform().data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "perspectiveTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, perspectiveMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "viewTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, viewingMatrix.data());

        glBindTexture(GL_TEXTURE_2D, mTexturesToRender[it.first]);
        glBindVertexArray(mVAO[it.first]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }

    deactivateShader();
}


} // end namespace fast
