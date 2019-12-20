#include "TextRenderer.hpp"
#include "FAST/Visualization/View.hpp"
#include <QPainter>
#include <QApplication>
#include <FAST/Data/Text.hpp>
#include <QScreen>

namespace fast {


BoundingBox TextRenderer::getBoundingBox(bool transform) {
    return BoundingBox(Vector3f(6,6,6));
}

TextRenderer::TextRenderer() {
    createInputPort<Text>(0);
    mStyle = STYLE_NORMAL;
    m_position = POSITION_CENTER;
    m_positionType = PositionType::STANDARD;
	mFontSize = 28;
	mColor = Color::Green();
    createStringAttribute("position", "Text position", "Position of text in view (center/bottom_left/bottom_right/top_left/top_right)", "top_left");
    createIntegerAttribute("font_size", "Font size", "Font size", mFontSize);

    createShaderProgram({
        Config::getKernelSourcePath() + "/Visualization/TextRenderer/TextRenderer.vert",
        Config::getKernelSourcePath() + "/Visualization/TextRenderer/TextRenderer.frag",
    });
}

void TextRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, float zNear, float zFar, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);
    if(!mode2D)
        throw Exception("TextRender is only implemented for 2D at the moment");

    for(auto it : mDataToRender) {
        auto input = std::static_pointer_cast<Text>(it.second);
        uint inputNr = it.first;

        // Check if a texture has already been created for this text
        if(mTexturesToRender.count(inputNr) > 0 && mTextUsed[inputNr] == input)
            continue; // If it has already been created, skip it

        if(mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
            glDeleteVertexArrays(1, &mVAO[inputNr]);
            mVAO.erase(inputNr);
        }

        std::string text = input->getText();

        if(text.empty()) {
            continue;
        }
            
        // Font setup
        auto selectedColor = input->getColor();
        QColor color(selectedColor.getRedValue() * 255, selectedColor.getGreenValue() * 255, selectedColor.getBlueValue() * 255, 255);
        QFont font = QApplication::font();
        font.setPixelSize(mFontSize);
        if (mStyle == STYLE_BOLD) {
            font.setBold(true);
        } else if (mStyle == STYLE_ITALIC) {
            font.setItalic(true);
        }
        QFontMetrics metrics(font);
        const int width = metrics.boundingRect(QString(text.c_str())).width()+3;
        const int height = metrics.boundingRect(QString(text.c_str())).height();
        
        // create the QImage and draw txt into it
        QImage textimg(width, height, QImage::Format_RGBA8888);
        textimg.fill(QColor(0, 0, 0, 0));
        {
            // Draw the text to the image
            QPainter painter(&textimg);
            painter.setBrush(color);
            painter.setPen(color);
            painter.setFont(font);
            painter.drawText(0, mFontSize, text.c_str());
        }

        // Make texture of QImage
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     textimg.width(), textimg.height(),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, textimg.bits());
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
        mTextUsed[inputNr] = input;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    activateShader();
    for(auto it : mTexturesToRender) {
        const uint inputNr = it.first;
        Affine3f transform = Affine3f::Identity();

        // Get width and height of texture
        glBindTexture(GL_TEXTURE_2D, mTexturesToRender[inputNr]);
        int width, height;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
        const float scale = 1.0f / m_view->width();
        const float scale2 = 1.0f / m_view->height();
        const int padding = 15;
        Vector3f position;
        if(m_positionType == PositionType::STANDARD) {
            switch(m_position) {
                case POSITION_CENTER:
                    position = Vector3f(-width * scale / 2.0f, -height * scale2 / 2.0f, 0); // Center
                    break;
                case POSITION_TOP_CENTER:
                    position = Vector3f(-width * scale / 2.0f, 1 - (height + padding) * scale2, 0); // Top center
                    break;
                case POSITION_BOTTOM_CENTER:
                    position = Vector3f(-width * scale / 2.0f, -1 + padding * scale2, 0); // Bottom center
                    break;
                case POSITION_TOP_LEFT:
                    position = Vector3f(-1 + padding * scale, 1 - (height + padding) * scale2, 0); // Top left corner
                    break;
                case POSITION_TOP_RIGHT:
                    position = Vector3f(1 - (width + padding) * scale, 1 - (height + padding) * scale2,
                                        0); // Top right corner
                    break;
                case POSITION_BOTTOM_LEFT:
                    position = Vector3f(-1 + padding * scale, -1 + padding * scale2, 0); // Bottom left corner
                    break;
                case POSITION_BOTTOM_RIGHT:
                    position = Vector3f(1 - (width + padding) * scale, -1 + padding * scale2, 0); // Bottom right corner
                    break;
            }
            transform.translate(position);
            transform.scale(Vector3f(scale, scale2, 0));
        } else if(m_positionType == PositionType::WORLD) {
            float textHeight = mTextUsed[inputNr]->getTextHeight();
            float textSpacing = textHeight/height;
            float textWidthInMM = textSpacing*width;
            auto T = SceneGraph::getEigenAffineTransformationFromData(mTextUsed[inputNr]);
            m_worldPosition.x() = T.translation().x();
            m_worldPosition.y() = T.translation().y();
            if(m_centerPosition) {
                position.x() = m_worldPosition.x() - textWidthInMM * 0.5f;
                position.y() = m_worldPosition.y() + textHeight * 0.5f;
            } else {
                position.x() = m_worldPosition.x();
                position.y() = m_worldPosition.y() + textHeight;
            }
            position.z() = 0;
            transform.translate(position);
            transform.scale(Vector3f(textSpacing, -1.0f*textSpacing, 1.0f));
        } else if(m_positionType == PositionType::VIEW) {
        }

        uint transformLoc = glGetUniformLocation(getShaderProgram(), "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "perspectiveTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, perspectiveMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "viewTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, viewingMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "positionType");
        glUniform1i(transformLoc, (int)m_positionType);

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

void TextRenderer::setFontSize(uint fontSize) {
	mFontSize = fontSize;
}

void TextRenderer::setPosition(TextRenderer::TextPosition position) {
    m_position = position;
    m_positionType = PositionType::STANDARD;
}

void TextRenderer::setColor(Color color) {
	mColor = color;
}

void TextRenderer::setStyle(TextStyleType textStyleType) {
	mStyle = textStyleType;
}

void TextRenderer::loadAttributes() {
    std::string position = getStringAttribute("position");
    if(position == "center") {
        setPosition(POSITION_CENTER);
    } else if(position == "top_left") {
        setPosition(POSITION_TOP_LEFT);
    } else if(position == "top_right") {
        setPosition(POSITION_TOP_RIGHT);
    } else if(position == "bottom_left") {
        setPosition(POSITION_BOTTOM_LEFT);
    } else if(position == "bottom_right") {
        setPosition(POSITION_BOTTOM_RIGHT);
    } else {
        throw Exception("Uknown position for TextRenderer: " + position);
    }
    setFontSize(getIntegerAttribute("font_size"));
}

void TextRenderer::setViewPosition(Vector2f position, float centerPosition) {
    m_viewPosition = position;
    m_centerPosition = centerPosition;
    m_positionType = PositionType::VIEW;
}

void TextRenderer::setWorldPosition(Vector2f position, float centerPosition) {
    m_worldPosition = position;
    m_centerPosition = centerPosition;
    m_positionType = PositionType::WORLD;
}

void TextRenderer::setFontHeightInMM(float heightInMillimeters) {
    m_textHeightInMM = heightInMillimeters;
}

void TextRenderer::setPositionType(PositionType position) {
    m_positionType = position;
}

} // end namespace fast


