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

void SegmentationRenderer::loadAttributes() {
    setOpacity(getFloatAttribute("opacity"));
    auto colors = getStringListAttribute("label-colors");
    for(int i = 0; i < colors.size(); i += 2) {
        int label = std::stoi(colors[i]);
        Color color = Color::fromString(colors.at(i + 1));
        setColor(label, color);
    }
}

SegmentationRenderer::SegmentationRenderer() {
    createInputPort<Image>(0, false);

    createFloatAttribute("opacity", "Segmentation Opacity", "", mOpacity);
    createStringAttribute("label-colors", "Label color", "Label color set as <label1> <color1> <label2> <color2>", "");

    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/SegmentationRenderer/SegmentationRenderer.frag",
                        }, "unsigned-integer");

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
    if(mDataToRender.empty())
        return;
    GLuint filterMethod = mUseInterpolation ? GL_LINEAR : GL_NEAREST;
    OpenCLDevice::pointer device = std::dynamic_pointer_cast<OpenCLDevice>(getMainDevice());

    createColorUniformBufferObject();

    // TODO move to func?
    auto colorsIndex = glGetUniformBlockIndex(getShaderProgram("unsigned-integer"), "Colors");
    glUniformBlockBinding(getShaderProgram("unsigned-integer"), colorsIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_colorsUBO);

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
        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
            glDeleteVertexArrays(1, &mVAO[inputNr]);
            mVAO.erase(inputNr);
        }

        auto access = input->getOpenGLTextureAccess(ACCESS_READ, device);
        auto textureID = access->get();
        mTexturesToRender[inputNr] = textureID;
        mImageUsed[inputNr] = input;
        mDataTimestamp[inputNr] = input->getTimestamp();
    }

    activateShader("unsigned-integer");
    setShaderUniform("opacity", mOpacity, "unsigned-integer");

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

void SegmentationRenderer::setInterpolation(bool useInterpolation) {
    mUseInterpolation = useInterpolation;
    deleteAllTextures();
}

}
