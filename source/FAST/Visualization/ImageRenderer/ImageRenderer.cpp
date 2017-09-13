#include "ImageRenderer.hpp"
#include "FAST/Exception.hpp"
#include "FAST/DeviceManager.hpp"
#include "FAST/Utility.hpp"
#include "FAST/SceneGraph.hpp"
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

#ifndef GL_RGBA32F // this is missing on windows and mac for some reason
#define GL_RGBA32F 0x8814 
#endif


ImageRenderer::ImageRenderer() : Renderer() {
    createInputPort<Image>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/ImageRenderer/ImageRenderer.cl", "3D");
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/ImageRenderer/ImageRenderer2D.cl", "2D");
    mIsModified = true;
    mWindow = -1;
    mLevel = -1;
    createFloatAttribute("window", "Intensity window", "Intensity window", -1);
    createFloatAttribute("level", "Intensity level", "Intensity level", -1);
}

void ImageRenderer::setIntensityLevel(float level) {
    mLevel = level;
}

float ImageRenderer::getIntensityLevel() {
    return mLevel;
}

void ImageRenderer::setIntensityWindow(float window) {
    if (window <= 0)
        throw Exception("Intensity window has to be above 0.");
    mWindow = window;
}

float ImageRenderer::getIntensityWindow() {
    return mWindow;
}

void ImageRenderer::loadAttributes() {
    setIntensityWindow(getFloatAttribute("window"));
    setIntensityLevel(getFloatAttribute("level"));
}

void ImageRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix) {

    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/ImageRenderer/ImageRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/ImageRenderer/ImageRenderer.frag",
                        });
    std::lock_guard<std::mutex> lock(mMutex);

    std::unordered_map<uint, Image::pointer>::iterator it;
    for(auto it : mDataToRender) {
        Image::pointer input = it.second;
        uint inputNr = it.first;

        // Check if a texture has already been created for this image
        if(mTexturesToRender.count(inputNr) > 0 && mImageUsed[inputNr] == input)
            continue; // If it has already been created, skip it

        // If it has not been created, create the texture

        // Determine level and window
        float window = mWindow;
        float level = mLevel;
        // If mWindow/mLevel is equal to -1 use default level/window values
        if(window == -1) {
            window = getDefaultIntensityWindow(input->getDataType());
        }
        if(level == -1) {
            level = getDefaultIntensityLevel(input->getDataType());
        }

        OpenCLDevice::pointer device = getMainDevice();

        OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
        cl::Image2D *clImage = access->get2DImage();

        mKernel = cl::Kernel(getOpenCLProgram(device, "3D"), "renderToTexture");
        // Run kernel to fill the texture
        cl::CommandQueue queue = device->getCommandQueue();

        //glEnable(GL_TEXTURE_2D);
        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
        }

        cl::Image2D image;
        cl::ImageGL imageGL;
        std::vector<cl::Memory> v;
        GLuint textureID;
        if(DeviceManager::isGLInteropEnabled()) {
            // Create OpenGL texture
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, clImage->getImageInfo<CL_IMAGE_WIDTH>(),
                         clImage->getImageInfo<CL_IMAGE_HEIGHT>(), 0, GL_RGBA, GL_FLOAT, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFinish();

            // Create CL-GL image
            imageGL = cl::ImageGL(
                    device->getContext(),
                    CL_MEM_READ_WRITE,
                    GL_TEXTURE_2D,
                    0,
                    textureID
            );
            mKernel.setArg(1, imageGL);
            v.push_back(imageGL);
            queue.enqueueAcquireGLObjects(&v);
        } else {
            image = cl::Image2D(
                    device->getContext(),
                    CL_MEM_READ_WRITE,
                    cl::ImageFormat(CL_RGBA, CL_FLOAT),
                    input->getWidth(), input->getHeight()
            );
            mKernel.setArg(1, image);
        }


        mKernel.setArg(0, *clImage);
        mKernel.setArg(2, level);
        mKernel.setArg(3, window);
        queue.enqueueNDRangeKernel(
                mKernel,
                cl::NullRange,
                cl::NDRange(clImage->getImageInfo<CL_IMAGE_WIDTH>(), clImage->getImageInfo<CL_IMAGE_HEIGHT>()),
                cl::NullRange
        );

        if(DeviceManager::isGLInteropEnabled()) {
            queue.enqueueReleaseGLObjects(&v);
        } else {
            // Copy data from CL image to CPU
            float *data = new float[input->getWidth() * input->getHeight() * 4];
            queue.enqueueReadImage(
                    image,
                    CL_TRUE,
                    createOrigoRegion(),
                    createRegion(input->getWidth(), input->getHeight(), 1),
                    0, 0,
                    data
            );
            // Copy data from CPU to GL texture
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, clImage->getImageInfo<CL_IMAGE_WIDTH>(),
                         clImage->getImageInfo<CL_IMAGE_HEIGHT>(), 0, GL_RGBA, GL_FLOAT, data);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFinish();
            delete[] data;
        }

        mTexturesToRender[inputNr] = textureID;
        mImageUsed[inputNr] = input;
        queue.finish();

        // Create VAO
        uint VAO_ID;
        glGenVertexArrays(1, &VAO_ID);
        mVAO[inputNr] = VAO_ID;
        glBindVertexArray(VAO_ID);

        // Create VBO
        // Get width and height in mm
        float width = input->getWidth() * input->getSpacing().x();
        float height = input->getHeight() * input->getSpacing().y();
        float vertices[] = {
                // vertex: x, y, z; tex coordinates: x, y
                0.0f, height, 0.0f, 0.0f, 0.0f,
                width, height, 0.0f, 1.0f, 0.0f,
                width, 0.0f, 0.0f, 1.0f, 1.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        };
        uint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // Create EBO
        uint EBO;
        glGenBuffers(1, &EBO);
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
    for(it = mImageUsed.begin(); it != mImageUsed.end(); it++) {
        glPushMatrix();

        AffineTransformation::pointer transform = SceneGraph::getAffineTransformationFromData(it->second);
        //glMultMatrixf(transform->getTransform().data());

        uint transformLoc = glGetUniformLocation(getShaderProgram(), "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform->getTransform().data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "perspectiveTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, perspectiveMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "viewTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, viewingMatrix.data());

        glBindTexture(GL_TEXTURE_2D, mTexturesToRender[it->first]);
        glBindVertexArray(mVAO[it->first]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
        glPopMatrix();
    }

    deactivateShader();
}

void ImageRenderer::draw2D(cl::Buffer PBO, uint width, uint height,
                           Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform, float PBOspacing,
                           Vector2f translation) {
    std::lock_guard<std::mutex> lock(mMutex);

    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    std::vector<cl::Memory> v;
    if(DeviceManager::isGLInteropEnabled()) {
        v.push_back(PBO);
        queue.enqueueAcquireGLObjects(&v);
    }

    // Create an aux PBO
    cl::Buffer PBO2(
            device->getContext(),
            CL_MEM_READ_WRITE,
            sizeof(float) * width * height * 4
    );

    for(auto it : mDataToRender) {
        Image::pointer input = it.second;
        // Determine level and window
        float window = mWindow;
        float level = mLevel;
        // If mWindow/mLevel is equal to -1 use default level/window values
        if(window == -1) {
            window = getDefaultIntensityWindow(input->getDataType());
        }
        if(level == -1) {
            level = getDefaultIntensityLevel(input->getDataType());
        }

        if(input->getDimensions() == 2) {
            cl::Kernel kernel(getOpenCLProgram(device, "2D"), "render2Dimage");
            // Run kernel to fill the texture

            OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
            cl::Image2D *clImage = access->get2DImage();
            kernel.setArg(0, *clImage);
            kernel.setArg(1, PBO); // Read from this
            kernel.setArg(2, PBO2); // Write to this
            kernel.setArg(3, input->getSpacing().x());
            kernel.setArg(4, input->getSpacing().y());
            kernel.setArg(5, PBOspacing);
            kernel.setArg(6, level);
            kernel.setArg(7, window);
            kernel.setArg(8, translation.x());
            kernel.setArg(9, translation.y());

            // Run the draw 2D kernel
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height),
                    cl::NullRange
            );
        } else {

            // Get transform of the image
            AffineTransformation::pointer dataTransform = SceneGraph::getAffineTransformationFromData(input);
            dataTransform->getTransform().scale(input->getSpacing()); // Apply image spacing

            // Transfer transformations
            Eigen::Affine3f transform = dataTransform->getTransform().inverse() * pixelToViewportTransform;

            cl::Buffer transformBuffer(
                    device->getContext(),
                    CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                    16 * sizeof(float),
                    transform.data()
            );

            cl::Kernel kernel(getOpenCLProgram(device, "2D"), "render3Dimage");
            // Run kernel to fill the texture

            OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
            cl::Image3D *clImage = access->get3DImage();
            kernel.setArg(0, *clImage);
            kernel.setArg(1, PBO); // Read from this
            kernel.setArg(2, PBO2); // Write to this
            kernel.setArg(3, transformBuffer);
            kernel.setArg(4, level);
            kernel.setArg(5, window);

            // Run the draw 3D image kernel
            device->getCommandQueue().enqueueNDRangeKernel(
                    kernel,
                    cl::NullRange,
                    cl::NDRange(width, height),
                    cl::NullRange
            );
        }

        // Copy PBO2 to PBO
        queue.enqueueCopyBuffer(PBO2, PBO, 0, 0, sizeof(float) * width * height * 4);
    }
    if(DeviceManager::isGLInteropEnabled()) {
        queue.enqueueReleaseGLObjects(&v);
    }
    queue.finish();
}

} // end namespace fast
