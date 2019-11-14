#include "SegmentationRenderer.hpp"



namespace fast {

void SegmentationRenderer::setColor(Segmentation::LabelType labelType,
        Color color) {
    mLabelColors[labelType] = color;
    mColorsModified = true;
    deleteAllTextures();
}

void SegmentationRenderer::setFillArea(Segmentation::LabelType labelType,
        bool fillArea) {
    mLabelFillArea[labelType] = fillArea;
    mFillAreaModified = true;
    deleteAllTextures();
}

void SegmentationRenderer::setFillArea(bool fillArea) {
    mFillArea = fillArea;
    deleteAllTextures();
}

SegmentationRenderer::SegmentationRenderer() {
    createInputPort<Image>(0, false);
    createOpenCLProgram(Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationRenderer.cl");
    mIsModified = false;
    mColorsModified = true;
    mFillAreaModified = true;
    mFillArea = true;

    // Set up default label colors
    mLabelColors[Segmentation::LABEL_BACKGROUND] = Color::Black();
    mLabelColors[Segmentation::LABEL_FOREGROUND] = Color::Green();
    mLabelColors[Segmentation::LABEL_BLOOD] = Color::Red();
    mLabelColors[Segmentation::LABEL_ARTERY] = Color::Red();
    mLabelColors[Segmentation::LABEL_VEIN] = Color::Blue();
    mLabelColors[Segmentation::LABEL_BONE] = Color::White();
    mLabelColors[Segmentation::LABEL_MUSCLE] = Color::Red();
    mLabelColors[Segmentation::LABEL_NERVE] = Color::Yellow();
    mLabelColors[Segmentation::LABEL_YELLOW] = Color::Yellow();
    mLabelColors[Segmentation::LABEL_GREEN] = Color::Green();
    mLabelColors[Segmentation::LABEL_MAGENTA] = Color::Magenta();
    mLabelColors[Segmentation::LABEL_RED] = Color::Red();
    mLabelColors[Segmentation::LABEL_WHITE] = Color::White();
    mLabelColors[Segmentation::LABEL_BLUE] = Color::Blue();
}

void
SegmentationRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());


    if(mColorsModified) {
        // Transfer colors to device (this doesn't have to happen every render call..)
        std::unique_ptr<float[]> colorData(new float[3*mLabelColors.size()]);
        std::unordered_map<int, Color>::iterator it;
        for(it = mLabelColors.begin(); it != mLabelColors.end(); it++) {
            colorData[it->first*3] = it->second.getRedValue();
            colorData[it->first*3+1] = it->second.getGreenValue();
            colorData[it->first*3+2] = it->second.getBlueValue();
        }

        mColorBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*3*mLabelColors.size(),
                colorData.get()
        );
    }

    if(mFillAreaModified) {
        // Transfer colors to device (this doesn't have to happen every render call..)
        std::unique_ptr<char[]> fillAreaData(new char[mLabelColors.size()]);
        std::unordered_map<int, Color>::iterator it;
        for(it = mLabelColors.begin(); it != mLabelColors.end(); it++) {
            if(mLabelFillArea.count(it->first) == 0) {
                // Use default value
                fillAreaData[it->first] = mFillArea;
            } else {
                fillAreaData[it->first] = mLabelFillArea[it->first];
            }
        }

        mFillAreaBuffer = cl::Buffer(
                device->getContext(),
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(char)*mLabelColors.size(),
                fillAreaData.get()
        );
    }

    mKernel = cl::Kernel(getOpenCLProgram(device), "renderToTexture");
    mKernel.setArg(2, mColorBuffer);
    mKernel.setArg(3, mFillAreaBuffer);
    mKernel.setArg(4, mBorderRadius);
    mKernel.setArg(5, mOpacity);


    for(auto it : mDataToRender) {
        Image::pointer input = std::static_pointer_cast<Image>(it.second);
        uint inputNr = it.first;

        if(input->getDimensions() != 2)
            throw Exception("SegmentationRenderer only supports 2D images. Use ImageSlicer to extract a 2D slice from a 3D image.");

        if(input->getDataType() != TYPE_UINT8)
            throw Exception("SegmentationRenderer only support images with dat type uint8.");

        // Check if a texture has already been created for this image
        if(mTexturesToRender.count(inputNr) > 0 && mImageUsed[inputNr] == input && mDataTimestamp[inputNr] == input->getTimestamp())
            continue; // If it has already been created, skip it

        // If it has not been created, create the texture

        OpenCLImageAccess::pointer access = input->getOpenCLImageAccess(ACCESS_READ, device);
        cl::Image2D *clImage = access->get2DImage();

        // Run kernel to fill the texture
        cl::CommandQueue queue = device->getCommandQueue();

        if (mTexturesToRender.count(inputNr) > 0) {
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
        // TODO The GL-CL interop here is causing glClear to not work on AMD systems and therefore disabled
        /*
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
            mKernel.setArg(1, imageGL);
            v.push_back(imageGL);
            queue.enqueueAcquireGLObjects(&v);
        } else {
         */
        image = cl::Image2D(
                device->getContext(),
                CL_MEM_READ_WRITE,
                cl::ImageFormat(CL_RGBA, CL_FLOAT),
                input->getWidth(), input->getHeight()
        );
        mKernel.setArg(1, image);
        //}


        mKernel.setArg(0, *clImage);
        queue.enqueueNDRangeKernel(
                mKernel,
                cl::NullRange,
                cl::NDRange(input->getWidth(), input->getHeight()),
                cl::NullRange
        );

        /*if(DeviceManager::isGLInteropEnabled()) {
            queue.enqueueReleaseGLObjects(&v);
        } else {*/
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
        //}

        mTexturesToRender[inputNr] = textureID;
        mImageUsed[inputNr] = input;
        mDataTimestamp[inputNr] = input->getTimestamp();
        queue.finish();
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawTextures(perspectiveMatrix, viewingMatrix, mode2D);
    glDisable(GL_BLEND);
}

void SegmentationRenderer::setBorderRadius(int radius) {
    if(radius <= 0)
        throw Exception("Border radius must be >= 0");

    mBorderRadius = radius;
    deleteAllTextures();
}

void SegmentationRenderer::setOpacity(float opacity) {
    if(opacity < 0 || opacity > 1)
        throw Exception("SegmentationRenderer opacity has to be >= 0 and <= 1");
    mOpacity = opacity;
    deleteAllTextures();
}

void SegmentationRenderer::setColor(int label, Color color) {
    mLabelColors[label] = color;
    mColorsModified = true;
    deleteAllTextures();
}

}
