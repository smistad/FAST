#include "SegmentationLabelRenderer.hpp"
#include "FAST/Visualization/View.hpp"
#include <QPainter>
#include <QApplication>
#include <FAST/Data/Image.hpp>
#include <QScreen>
#include <FAST/Algorithms/RegionProperties/RegionProperties.hpp>

namespace fast {

SegmentationLabelRenderer::SegmentationLabelRenderer() {
    createInputPort<Image>(0);
    mFontSize = 28;

    createShaderProgram({
                                Config::getKernelSourcePath() + "/Visualization/SegmentationLabelRenderer/SegmentationLabelRenderer.vert",
                                Config::getKernelSourcePath() + "/Visualization/SegmentationLabelRenderer/SegmentationLabelRenderer.frag",
                        });
}


void SegmentationLabelRenderer::execute() {
    std::unique_lock<std::mutex> lock(mMutex);
    if(m_disabled)
        return;
    if(mStop) {
        return;
    }

    // Check if current images has not been rendered, if not wait
    while(!mHasRendered && m_synchedRendering) {
        mRenderedCV.wait(lock);
    }
    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputConnections(); inputNr++) {
        if(hasNewInputData(inputNr)) {
            SpatialDataObject::pointer input = getInputData<SpatialDataObject>(inputNr);

            mHasRendered = false;
            mDataToRender[inputNr] = input;

            auto regionProps = RegionProperties::New();
            regionProps->setInputData(input);
            m_regions[inputNr] = regionProps->updateAndGetOutputData<RegionList>();
        }
    }
}

void SegmentationLabelRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);
    if(!mode2D)
        throw Exception("SegmentationLabelRenderer is only implemented for 2D at the moment");

    for(auto it : mDataToRender) {
        auto input = std::static_pointer_cast<Image>(it.second);
        if(input->getDataType() != TYPE_UINT8)
            throw Exception("Input to SegmentationLabelRenderer must be image of type UINT8");

        uint inputNr = it.first;

        // Check if a texture has already been created for image
        if(mTexturesToRender.count(inputNr) > 0 && mImageUsed[inputNr] == input && mDataTimestamp[inputNr] == input->getTimestamp())
            continue;

        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
            glDeleteVertexArrays(1, &mVAO[inputNr]);
            mVAO.erase(inputNr);
        }


        // Font setup
        // Draw the text image with a larger resolution than the input if needed
        int width = input->getWidth();
        float scale = 1.0f;
        if(width < 1024) {
            scale = 1024.0f/width;
            width = 1024;
        }
        mScales[inputNr] = scale;
        int height = scale*input->getHeight();
        Vector3f spacing = input->getSpacing();

        // create the QImage and draw txt into it
        QImage textimg(width, height, QImage::Format_RGBA8888);
        textimg.fill(QColor(0, 0, 0, 0));
        QPainter painter(&textimg);
        float pixelArea = spacing.x()*spacing.y();
        for(auto& region : m_regions[inputNr]->getAccess(ACCESS_READ)->getData()) {

            if(region.area*pixelArea < 1.0f)
                continue;

            // If no label name has been set. Skip it
            if(m_labelNames.count(region.label) == 0)
                continue;

            float size = 1.0f;
            if(m_dynamicSize) {
                size = 1.0f + 0.01f*(region.area*pixelArea - 1.0f);
            } else {
                size = m_textHeightInMM;
            }

            const int textHeightInPixels = size / (spacing.y()/scale);

            QFont font = QApplication::font();
            font.setPixelSize(textHeightInPixels);
            QFontMetrics metrics(font);

            // Set label text
            std::string text = m_labelNames[region.label];

            // Get size of text
            const int textWidth = metrics.boundingRect(QString(text.c_str())).width()+3;
            const int textHeight = metrics.boundingRect(QString(text.c_str())).height();

            // Set color
            auto selectedColor = Color::Green();
            if(m_labelColors.count(region.label) > 0)
                selectedColor = m_labelColors[region.label];
            QColor color(selectedColor.getRedValue() * 255, selectedColor.getGreenValue() * 255, selectedColor.getBlueValue() * 255, 255);

            // Determine position
            Vector2f position;
            if(m_centerPosition) {
                position.x() = region.centroid.x()*scale - textWidth * 0.5f;
                position.y() = region.centroid.y()*scale + textHeight * 0.5f;
            } else {
                position.x() = region.centroid.x()*scale;
                position.y() = region.centroid.y()*scale + textHeight;
            }

            // Draw it
            painter.setBrush(color);
            painter.setPen(color);
            painter.setFont(font);
            painter.drawText(position.x(), position.y(), text.c_str());
        }
        QImage textimgFlipped = textimg.mirrored();

        // Make texture of QImage
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     textimg.width(), textimg.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, textimgFlipped.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Draw texture on a quad
        // Delete old VAO
        if(mVAO.count(inputNr) > 0)
            glDeleteVertexArrays(1, &mVAO[inputNr]);
        GLuint VAO_ID;
        glGenVertexArrays(1, &VAO_ID);
        mVAO[inputNr] = VAO_ID;
        glBindVertexArray(VAO_ID);
        float vertices[] = {
                // vertex: x, y, z; tex coordinates: x, y
                0.0f, (float)height, 0.0f, 0.0f, 0.0f,
                (float)width, (float)height, 0.0f, 1.0f, 0.0f,
                (float)width, 0.0f, 0.0f, 1.0f, 1.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        };

        // Delete old VBO
        if(mVBO.count(inputNr) > 0)
            glDeleteBuffers(1, &mVBO[inputNr]);
        // Create VBO
        uint VBO;
        glGenBuffers(1, &VBO);
        mVBO[inputNr] = VBO;
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
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

        mTexturesToRender[inputNr] = textureID;
        mImageUsed[inputNr] = input;
        mDataTimestamp[inputNr] = input->getTimestamp();
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    activateShader();
    for(auto it : mImageUsed) {
        const uint inputNr = it.first;

        AffineTransformation::pointer transform;
        if(mode2D) {
            // If rendering is in 2D mode we skip any transformations
            transform = AffineTransformation::New();
        } else {
            transform = SceneGraph::getAffineTransformationFromData(it.second);
        }

        transform->getTransform().scale(it.second->getSpacing()/mScales[inputNr]);

        // Get width and height of texture
        glBindTexture(GL_TEXTURE_2D, mTexturesToRender[inputNr]);
        uint transformLoc = glGetUniformLocation(getShaderProgram(), "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform->getTransform().data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "perspectiveTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, perspectiveMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "viewTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, viewingMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "positionType");

        // Actual drawing
        glBindVertexArray(mVAO[inputNr]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }
    glDisable(GL_BLEND);
    deactivateShader();
    glFinish(); // Fixes random crashes in OpenGL on NVIDIA windows due to some interaction with the line renderer. Suboptimal solution as glFinish is a blocking sync operation.
}

void SegmentationLabelRenderer::setLabelName(int label, std::string name) {
    m_labelNames[label] = name;
}

void SegmentationLabelRenderer::setLabelColor(int label, Color color) {
    m_labelColors[label] = color;
}


} // end namespace fast


