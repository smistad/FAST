#include "HeatmapRenderer.hpp"
#include "FAST/Data/Image.hpp"

namespace fast {

HeatmapRenderer::HeatmapRenderer() {
    createInputPort<Image>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/HeatmapRenderer/HeatmapRenderer.cl");
    mIsModified = false;
}

uint HeatmapRenderer::addInputConnection(DataPort::pointer port, Color color) {
    uint nr = Renderer::addInputConnection(port);
    mColors[nr] = color;
    return nr;
}

void HeatmapRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
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

    cl::Kernel kernel(getOpenCLProgram(device), "renderToTexture");
    for(auto it : mDataToRender) {
        Image::pointer input = std::static_pointer_cast<Image>(it.second);
        uint inputNr = it.first;

        if(input->getDataType() != TYPE_FLOAT) {
            throw Exception("Data type of image given to HeatmapRenderer must be FLOAT");
        }

        if(input->getDimensions() != 2)
            throw Exception("Image given to HeatmapRenderer must be 2D");

        // Run kernel to fill the texture
        Color color = colorList[it.first % colorList.size()];
        if(mColors.count(it.first) > 0) // has color
            color = mColors[it.first];

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
        if(DeviceManager::isGLInteropEnabled()) {
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
        kernel.setArg(2, color.getRedValue());
        kernel.setArg(3, color.getGreenValue());
        kernel.setArg(4, color.getBlueValue());
        kernel.setArg(5, mMinConfidence);
        kernel.setArg(6, mMaxOpacity);

        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(input->getWidth(), input->getHeight()),
            cl::NullRange
        );

        if(DeviceManager::isGLInteropEnabled()) {
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
    drawTextures(perspectiveMatrix, viewingMatrix, mode2D);
    glDisable(GL_BLEND);

}

void HeatmapRenderer::setMinConfidence(float confidence) {
    if(confidence < 0 || confidence > 1)
        throw Exception("Confidence given to setMinimumConfidence has to be within [0, 1]", __LINE__, __FILE__);
    mMinConfidence = confidence;
}

void HeatmapRenderer::setMaxOpacity(float opacity) {
    if(opacity < 0 || opacity > 1)
        throw Exception("Opacity given to setMaxOpacity has to be within [0, 1]", __LINE__, __FILE__);
    mMaxOpacity = opacity;
}

}