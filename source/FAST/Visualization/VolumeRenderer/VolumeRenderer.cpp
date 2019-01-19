#include "VolumeRenderer.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Utility.hpp>

namespace fast {

cl::Image2D VolumeRenderer::textureToCLimage(uint textureID, int width, int height, OpenCLDevice::pointer device, bool depth) {
    // TODO CL-GL interop
    int totalSize = width * height;
    if(!depth)
        totalSize *= 4;

    auto data = make_uninitialized_unique<float[]>(totalSize);
    glBindTexture(GL_TEXTURE_2D, textureID);
    if(depth) {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data.get());
    } else {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.get());
    }
    auto image = cl::Image2D(
        device->getContext(),
        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        depth ? cl::ImageFormat(CL_R, CL_FLOAT) : cl::ImageFormat(CL_RGBA, CL_FLOAT),
        width, height, 0,
        data.get()
    );

    return image;
}


cl::ImageGL VolumeRenderer::textureToCLimageInterop(uint textureID, int width, int height, OpenCLDevice::pointer device, bool depth) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glFinish();
    // Create CL-GL image
    auto imageGL = cl::ImageGL(
            device->getContext(),
            CL_MEM_READ_ONLY,
            GL_TEXTURE_2D,
            0,
            textureID
    );

    return imageGL;
}

std::tuple<uint, uint> VolumeRenderer::resizeOpenGLTexture(int sourceFBO, int sourceTextureColor, int sourceTextureDepth, Vector2i gridSize, int width, int height) {
    uint targetFBO;
    glGenFramebuffers(1, &targetFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    uint colorTexture, depthTexture;
    glGenTextures(1, &colorTexture);
    glGenTextures(1, &depthTexture);
    // Transfer texture data
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gridSize.x(), gridSize.y(), 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, gridSize.x(), gridSize.y(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Set texture to FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, gridSize.x(), gridSize.y(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glFinish();
    glDeleteFramebuffers(1, &targetFBO);

    return std::make_tuple(colorTexture, depthTexture);

}

void VolumeRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) {
    // Get window/viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    const float aspectRatio = (float)viewport[2] / viewport[3];
    const Vector2i gridSize(aspectRatio*1024, 1024);

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto queue = device->getCommandQueue();
    auto mKernel = cl::Kernel(getOpenCLProgram(device), "volumeRender");

    // Get color data from the main FBO to use as input
    int mainFBO;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &mainFBO);
    int colorTextureID, depthTextureID;
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &colorTextureID);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &depthTextureID);

    // Resize OpenGL textures to avoid issues when viewport is very large (4k screens for instance)
    // This also deals with issues related to gridSize being different than the viewport size giving problems when rendering geometry
    auto newTextures = resizeOpenGLTexture(mainFBO, colorTextureID, depthTextureID, gridSize, viewport[2], viewport[3]);
    colorTextureID = std::get<0>(newTextures);
    depthTextureID = std::get<1>(newTextures);

    std::vector<cl::Memory> v;
    // Image objects must exist until kernel has executed
    cl::Image2D inputColor;
    cl::Image2D inputDepth;
    cl::ImageGL inputColorGL;

    if(DeviceManager::isGLInteropEnabled() && false) { // TODO not working on AMD for some reason
        inputColorGL = textureToCLimageInterop(colorTextureID, gridSize.x(), gridSize.y(), device, false);
        //cl::ImageGL inputDepth = textureToCLimageInterop(depthTextureID, viewport[2], viewport[3], device, true); // Can't to interop on depth texture..
        inputDepth = textureToCLimage(depthTextureID, gridSize.x(), gridSize.y(), device, true);
        mKernel.setArg(9, inputColor);
        mKernel.setArg(10, inputDepth);
        v.push_back(inputColorGL);
        queue.enqueueAcquireGLObjects(&v);
    } else {
        inputColor = textureToCLimage(colorTextureID, gridSize.x(), gridSize.y(), device, false);
        inputDepth = textureToCLimage(depthTextureID, gridSize.x(), gridSize.y(), device, true);
        mKernel.setArg(9, inputColor);
        mKernel.setArg(10, inputDepth);
    }
    glDeleteTextures(1, (uint*)&colorTextureID);
    glDeleteTextures(1, (uint*)&depthTextureID);

    // Create a FBO
    if(m_FBO == 0)
        glGenFramebuffers(1, &m_FBO);

    // Bind the framebuffer to render to it
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
    // TODO CL-GL interop

    auto image = cl::Image2D(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_RGBA, CL_FLOAT),
            gridSize.x(), gridSize.y()
    );
    mKernel.setArg(1, image);

    auto input = getInputData<Image>(0);
    auto access = input->getOpenCLImageAccess(ACCESS_READ, device);
    cl::Image3D *clImage = access->get3DImage();

    float density = 0.05f;
    float brightness = 1.0f;
    float transferOffset = 0.0f;
    float transferScale = 1.0f;

    // create transfer function texture
    float transferFunc[] = {
            0.0, 0.0, 0.0, 0.0,
            1.0, 0.0, 0.0, 1.0,
            1.0, 0.5, 0.0, 1.0,
            1.0, 1.0, 0.0, 1.0,
            0.0, 1.0, 0.0, 1.0,
            0.0, 1.0, 1.0, 1.0,
            0.0, 0.0, 1.0, 1.0,
            1.0, 0.0, 1.0, 1.0,
            0.0, 0.0, 0.0, 0.0,
    };

    auto d_transferFuncArray = cl::Image2D(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            cl::ImageFormat(CL_RGBA, CL_FLOAT),
            9, 1, 0,
            transferFunc
    );

    Affine3f modelMatrix = SceneGraph::getEigenAffineTransformationFromData(input);
    modelMatrix.scale(input->getSpacing());
    Matrix4f invViewMatrix = (viewingMatrix*modelMatrix.matrix()).inverse();

    auto inverseViewMatrixBuffer = cl::Buffer(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            16*sizeof(float),
            invViewMatrix.data()
    );
    // TODO probably don't need this:
    auto modelMatrixBuffer = cl::Buffer(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            16*sizeof(float),
            modelMatrix.data()
    );

    mKernel.setArg(0, *clImage);
    mKernel.setArg(2, d_transferFuncArray);
    mKernel.setArg(3, density);
    mKernel.setArg(4, brightness);
    mKernel.setArg(5, transferOffset);
    mKernel.setArg(6, transferScale);
    mKernel.setArg(7, inverseViewMatrixBuffer);
    mKernel.setArg(8, modelMatrixBuffer);
    queue.enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            cl::NDRange(gridSize.x(), gridSize.y()),
            cl::NullRange
    );

    if(DeviceManager::isGLInteropEnabled() && false) {
        queue.enqueueReleaseGLObjects(&v);
    }

    // Attach texture to framebuffer
    if(m_texture == 0)
        glGenTextures(1, &m_texture);

    auto data = make_uninitialized_unique<float[]>(gridSize.x()*gridSize.y()*4);
    queue.enqueueReadImage(
            image,
            CL_TRUE,
            createOrigoRegion(),
            createRegion(gridSize.x(), gridSize.y(), 1),
            0, 0,
            data.get()
    );

    // Transfer texture data
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gridSize.x(), gridSize.y(), 0, GL_RGBA, GL_FLOAT, data.get());

    // Set texture to FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    // Blit/copy the framebuffer to the default framebuffer (window)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainFBO);
    glBlitFramebuffer(0, 0, gridSize.x(), gridSize.y(), viewport[0], viewport[1], viewport[2], viewport[3], GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Reset framebuffer to default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainFBO);
}

VolumeRenderer::~VolumeRenderer() {
    if(m_FBO != -1)
        glDeleteFramebuffers(1, &m_FBO);
    if(m_texture != -1)
        glDeleteFramebuffers(1, &m_texture);
}

VolumeRenderer::VolumeRenderer() {
    createInputPort<Image>(0);

    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VolumeRenderer/VolumeRenderer.cl");
}

}