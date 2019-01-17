#include "VolumeRenderer.hpp"
#include <FAST/Data/Image.hpp>
#include <FAST/Utility.hpp>

namespace fast {

void VolumeRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) {
    reportInfo() << "Draw in volume renderer" << reportEnd();

    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());
    auto queue = device->getCommandQueue();

    // Create a FBO
    if(m_FBO == -1)
        glGenFramebuffers(1, &m_FBO);

    // Bind the framebuffer to render to it
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
    // TODO CL-GL interop

    auto image = cl::Image2D(
            device->getContext(),
            CL_MEM_READ_WRITE,
            cl::ImageFormat(CL_RGBA, CL_FLOAT),
            512, 512
    );
    auto mKernel = cl::Kernel(getOpenCLProgram(device), "volumeRender");
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
    std::cout << "Model matrix:" << modelMatrix.matrix() << std::endl;
    Matrix4f invViewMatrix = (viewingMatrix).inverse();
    //modelView(3) = modelView(12);
    //modelView(7) = modelView(13);
    //modelView(11) = modelView(14);
    /*
    MatrixXf invViewMatrix = MatrixXf::Zero(3, 4);
    invViewMatrix(0) = modelView(0);
    invViewMatrix(1) = modelView(4);
    invViewMatrix(2) = modelView(8);
    invViewMatrix(3) = modelView(12);
    invViewMatrix(4) = modelView(1);
    invViewMatrix(5) = modelView(5);
    invViewMatrix(6) = modelView(9);
    invViewMatrix(7) = modelView(13);
    invViewMatrix(8) = modelView(2);
    invViewMatrix(9) = modelView(6);
    invViewMatrix(10) = modelView(10);
    invViewMatrix(11) = modelView(14);
     */
    auto inverseViewMatrixBuffer = cl::Buffer(
            device->getContext(),
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            16*sizeof(float),
            invViewMatrix.data()
    );
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
            cl::NDRange(512, 512),
            cl::NullRange
    );


    // Attach texture to framebuffer
    if(m_texture == -1)
        glGenTextures(1, &m_texture);

    auto data = make_uninitialized_unique<float[]>(512*512*4);
    queue.enqueueReadImage(
            image,
            CL_TRUE,
            createOrigoRegion(),
            createRegion(512, 512, 1),
            0, 0,
            data.get()
    );
    /*
    for(int i = 0; i < 512*512*4; i++) {
        if(data[i] > 0) {
            std::cout << data[i] << std::endl;
        }
    }
     */

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_FLOAT, data.get());
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    // Blit/copy the framebuffer to the default framebuffer (window)
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Get window/viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glBlitFramebuffer(0, 0, 512, 512, viewport[0], viewport[1], viewport[2], viewport[3], GL_COLOR_BUFFER_BIT, GL_LINEAR);
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