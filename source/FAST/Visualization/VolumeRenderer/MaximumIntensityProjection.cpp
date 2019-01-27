#include "MaximumIntensityProjection.hpp"
#include <FAST/Data/Image.hpp>

namespace fast {

void MaximumIntensityProjection::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) {
    // Get window/viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    const float aspectRatio = (float)viewport[2] / viewport[3];
    const int height = std::min(1024, viewport[3]);
    const Vector2i gridSize(aspectRatio*height, height);

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

	bool useGLInterop = false;
    if(DeviceManager::isGLInteropEnabled()) {		
		try {
			inputColorGL = textureToCLimageInterop(colorTextureID, gridSize.x(), gridSize.y(), device, false);
			v.push_back(inputColorGL);
			queue.enqueueAcquireGLObjects(&v);
			//cl::ImageGL inputDepth = textureToCLimageInterop(depthTextureID, viewport[2], viewport[3], device, true); // Can't to interop on depth texture..
			inputDepth = textureToCLimage(depthTextureID, gridSize.x(), gridSize.y(), device, true);
			mKernel.setArg(4, inputColorGL);
			mKernel.setArg(5, inputDepth);
			useGLInterop = true;
		} catch (cl::Error &e) {
			reportError() << "Failed to perform GL interop in volume renderer even though it is enabled on device." << reportEnd();
		}
    } 

	if(!useGLInterop) {
        inputColor = textureToCLimage(colorTextureID, gridSize.x(), gridSize.y(), device, false);
        inputDepth = textureToCLimage(depthTextureID, gridSize.x(), gridSize.y(), device, true);
        mKernel.setArg(4, inputColor);
        mKernel.setArg(5, inputDepth);
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
    mKernel.setArg(2, inverseViewMatrixBuffer);
    mKernel.setArg(3, modelMatrixBuffer);
    queue.enqueueNDRangeKernel(
            mKernel,
            cl::NullRange,
            cl::NDRange(gridSize.x(), gridSize.y()),
            cl::NullRange
    );

    if(useGLInterop) {
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

MaximumIntensityProjection::MaximumIntensityProjection() {
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/VolumeRenderer/MaximumIntensityProjection.cl");
}

}