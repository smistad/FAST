#include "HeatmapRenderer.hpp"
#include <FAST/Data/Tensor.hpp>
#include <FAST/Data/Access/OpenCLBufferAccess.hpp>

namespace fast {

HeatmapRenderer::HeatmapRenderer() {
    createInputPort<Tensor>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/HeatmapRenderer/HeatmapRenderer.cl");
    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/ImageRenderer/ImageRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/ImageRenderer/ImageRenderer.frag",
                        });
    mIsModified = false;
    mColorsModified = true;
}

void HeatmapRenderer::setChannelColor(uint channel, Color color) {
    mColors[channel] = color;
    mColorsModified = true;
    deleteAllTextures();
}

void HeatmapRenderer::setChannelHidden(uint channel, bool hide) {
    mHide[channel] = hide;
    mColorsModified = true;
    deleteAllTextures();
}

void HeatmapRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    if(mDataToRender.empty())
        return;
    GLuint filterMethod = mUseInterpolation ? GL_LINEAR : GL_NEAREST;
    std::lock_guard<std::mutex> lock(mMutex);
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();

    std::vector<Color> colorList = {
        Color::Green(),
        Color::Blue(),
        Color::Red(),
        Color::Magenta(),
        Color::Yellow(),
        Color::Cyan(),
    };
    int maxChannels = 0;
    for(auto it : mDataToRender) {
        auto input = std::static_pointer_cast<Tensor>(it.second);
        int nrOfChannels = input->getShape()[2];
        maxChannels = std::max(nrOfChannels, maxChannels);
    }

    if(mColorsModified && maxChannels > 0) {
        // Transfer colors to device (this doesn't have to happen every render call..)
        auto colorData = make_uninitialized_unique<float[]>(4*maxChannels);
        Color defaultColor = Color::Green();
        for(int i = 0; i < maxChannels; ++i) {
            if(mColors.count(i) > 0) {
                colorData[i * 4] = mColors[i].getRedValue();
                colorData[i * 4 + 1] = mColors[i].getGreenValue();
                colorData[i * 4 + 2] = mColors[i].getBlueValue();
            } else if(colorList.size() > i) {
                colorData[i * 4] = colorList[i].getRedValue();
                colorData[i * 4 + 1] = colorList[i].getGreenValue();
                colorData[i * 4 + 2] = colorList[i].getBlueValue();
            } else {
                colorData[i*4] = defaultColor.getRedValue();
                colorData[i*4 + 1] = defaultColor.getGreenValue();
                colorData[i*4 + 2] = defaultColor.getBlueValue();
            }
            if(mHide.count(i) > 0 && mHide[i]) {
                // If channel should be hidden; set it to white, and alpha = 0
                colorData[i * 4 + 0] = 1.0f;
                colorData[i * 4 + 1] = 1.0f;
                colorData[i * 4 + 2] = 1.0f;
                colorData[i * 4 + 3] = 0.0f;
            } else {
                colorData[i * 4 + 3] = 1.0f;
            }
        }

        mColorBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*4*maxChannels,
                colorData.get()
        );
		mColorsModified = false;
    }

    cl::Kernel kernel(getOpenCLProgram(device), "renderToTexture");
    for(auto it : mDataToRender) {
        auto input = std::static_pointer_cast<Tensor>(it.second);
        uint inputNr = it.first;

        if(input->getShape().getDimensions() != 3)
            throw Exception("Tensor given to HeatmapRenderer must be 3D (width x height x channels), actual shape: " + input->getShape().toString());

        // Check if a texture has already been created for this image
        if(mTexturesToRender.count(inputNr) > 0 && mTensorUsed[inputNr] == input && mDataTimestamp[inputNr] == input->getTimestamp())
            continue; // If it has already been created, skip it

        const int width = input->getShape()[1];
        const int height = input->getShape()[0];

        // Run kernel to fill the texture

        auto access = input->getOpenCLBufferAccess(ACCESS_READ, device);

        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
            glDeleteVertexArrays(1, &mVAO[inputNr]);
            mVAO.erase(inputNr);
        }

        cl::Image2D image;
        cl::ImageGL imageGL;
        std::vector<cl::Memory> v;
        GLuint textureID;
        if(DeviceManager::isGLInteropEnabled()) {
            // Create OpenGL texture
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMethod);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMethod);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);

            // Create CL-GL image
            imageGL = cl::ImageGL(
                    device->getContext(),
                    CL_MEM_READ_WRITE,
                    GL_TEXTURE_2D,
                    0,
                    textureID
            );
            glBindTexture(GL_TEXTURE_2D, 0);
            glFinish();
            v.push_back(imageGL);
            queue.enqueueAcquireGLObjects(&v);
            kernel.setArg(1, imageGL);
        } else {
            image = cl::Image2D(
                    device->getContext(),
                    CL_MEM_READ_WRITE,
                    cl::ImageFormat(CL_RGBA, CL_FLOAT),
                    width, height
            );
            kernel.setArg(1, image);
        }

        kernel.setArg(0, *access->get());
        kernel.setArg(2, mColorBuffer);
        kernel.setArg(3, mMinConfidence);
        kernel.setArg(4, mMaxOpacity);
        kernel.setArg(5, input->getShape()[2]);

        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(width, height),
            cl::NullRange
        );

        if(DeviceManager::isGLInteropEnabled()) {
            queue.enqueueReleaseGLObjects(&v);
            queue.finish();
        } else {
            // Copy data from CL image to CPU
            auto data = make_uninitialized_unique<float[]>(width * height * 4);
            queue.enqueueReadImage(
                    image,
                    CL_TRUE,
                    createOrigoRegion(),
                    createRegion(width, height, 1),
                    0, 0,
                    data.get()
            );
            // Copy data from CPU to GL texture
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMethod);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMethod);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data.get());
            glBindTexture(GL_TEXTURE_2D, 0);
            glFinish();
        }

        mTexturesToRender[inputNr] = textureID;
        mTensorUsed[inputNr] = input;
        mDataTimestamp[inputNr] = input->getTimestamp();
        queue.finish();
    }

    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawTextures(perspectiveMatrix, viewingMatrix, mode2D);
    glDisable(GL_BLEND);

}

void HeatmapRenderer::drawTextures(Matrix4f &perspectiveMatrix, Matrix4f &viewingMatrix, bool mode2D) {

    for(auto it : mDataToRender) {
        auto input = std::static_pointer_cast<Tensor>(it.second);
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
        float width = input->getShape()[1];// * input->getSpacing().x();
        float height = input->getShape()[0];// * input->getSpacing().y();
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
    for(auto& it : mTensorUsed) {
        AffineTransformation::pointer transform;
        if(mode2D) {
            // If rendering is in 2D mode we skip any transformations
            transform = AffineTransformation::New();
        } else {
            transform = SceneGraph::getAffineTransformationFromData(it.second);
        }

        Vector3f spacing = it.second->getSpacing();
        transform->getTransform().scale(spacing);

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

void HeatmapRenderer::setMinConfidence(float confidence) {
    if(confidence < 0 || confidence > 1)
        throw Exception("Confidence given to setMinimumConfidence has to be within [0, 1]", __LINE__, __FILE__);
    mMinConfidence = confidence;
    deleteAllTextures();
}

void HeatmapRenderer::setMaxOpacity(float opacity) {
    if(opacity < 0 || opacity > 1)
        throw Exception("Opacity given to setMaxOpacity has to be within [0, 1]", __LINE__, __FILE__);
    mMaxOpacity = opacity;
    deleteAllTextures();
}

void HeatmapRenderer::setInterpolation(bool useInterpolation) {
    mUseInterpolation = useInterpolation;
    deleteAllTextures();
}

}