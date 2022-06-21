#include "VectorFieldColorRenderer.hpp"

namespace fast {

VectorFieldColorRenderer::VectorFieldColorRenderer(float maxOpacity, float maxLength) {
    m_2Donly = true;
    createInputPort(0, "Image", "", false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VectorFieldRenderer/VectorFieldColorRenderer.cl");
    mIsModified = false;
    setMaxOpacity(maxOpacity);
    setMaxLength(maxLength);
}

void
VectorFieldColorRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D,
                               int viewWidth,
                               int viewHeight) {
    auto dataToRender = getDataToRender();
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    cl::CommandQueue queue = device->getCommandQueue();

    cl::Kernel kernel(getOpenCLProgram(device), "renderToTexture");
    for(auto it : dataToRender) {
        Image::pointer input = std::static_pointer_cast<Image>(it.second);
        uint inputNr = it.first;
        float maxComponent;
        if(m_maxLength > 0) {
            maxComponent = m_maxLength;
        } else {
            maxComponent = input->calculateMaximumIntensity();
        }

        if(input->getDataType() != TYPE_FLOAT) {
            throw Exception("Data type of image given to VectorFieldColorRenderer must be FLOAT");
        }

        if(input->getDimensions() != 2)
            throw Exception("Image given to VectorFieldColorRenderer must be 2D");

        OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
        cl::Image2D *clImage = access->get2DImage();

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
        if(device->isOpenGLInteropSupported()) {
            // Create OpenGL texture
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, input->getWidth(), input->getHeight(), 0, GL_RGBA, GL_FLOAT, 0);

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
                    input->getWidth(), input->getHeight()
            );
            kernel.setArg(1, image);
        }

        kernel.setArg(0, *clImage);
        kernel.setArg(2, m_maxOpacity);
        kernel.setArg(3, 1.4f*maxComponent); // sqrt(max*max + max*max) = sqrt(2*max*max) = 1.4*max

        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
        );

        if(device->isOpenGLInteropSupported()) {
            queue.enqueueReleaseGLObjects(&v);
            queue.finish();
        } else {
            // Copy data from CL image to CPU
            auto data = make_uninitialized_unique<float[]>(input->getWidth() * input->getHeight() * 4);
            queue.enqueueReadImage(
                    image,
                    CL_TRUE,
                    createOrigoRegion(),
                    createRegion(input->getWidth(), input->getHeight(), 1),
                    0, 0,
                    data.get()
            );
            // Copy data from CPU to GL texture
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, input->getWidth(), input->getHeight(), 0, GL_RGBA, GL_FLOAT, data.get());
            glBindTexture(GL_TEXTURE_2D, 0);
            glFinish();
        }

        mTexturesToRender[inputNr] = textureID;
        mImageUsed[inputNr] = input;
        queue.finish();
    }

    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawTextures(dataToRender, perspectiveMatrix, viewingMatrix, mode2D);
    glDisable(GL_BLEND);
}

void VectorFieldColorRenderer::setMaxOpacity(float maxOpacity) {
    if(maxOpacity <= 0.0f || maxOpacity > 1.0f)
        throw Exception("Max opacity must be within (0.0, 1.0]");

    m_maxOpacity = maxOpacity;
}

void VectorFieldColorRenderer::setMaxLength(float max) {
    m_maxLength = max;
}

}