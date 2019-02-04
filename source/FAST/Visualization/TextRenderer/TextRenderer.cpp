
#include "TextRenderer.hpp"
#include "FAST/Visualization/View.hpp"
#include <QPainter>
#include <QApplication>
#include <FAST/Data/Text.hpp>

namespace fast {


BoundingBox TextRenderer::getBoundingBox(bool transform) {
    return BoundingBox(Vector3f(6,6,6));
}

TextRenderer::TextRenderer() {
    createInputPort<Text>(0);
    mStyle = STYLE_NORMAL;
    mPosition = POSITION_CENTER;
	mFontSize = 18;
	mColor = Color::Green();
    createStringAttribute("position", "Text position", "Position of text in view (center/bottom_left/bottom_right/top_left/top_right)", "top_left");
    createIntegerAttribute("font_size", "Font size", "Font size", mFontSize);

    createShaderProgram({
        Config::getKernelSourcePath() + "/Visualization/TextRenderer/TextRenderer.vert",
        Config::getKernelSourcePath() + "/Visualization/TextRenderer/TextRenderer.frag",
    });
}

void TextRenderer::setView(View* view) {
	mView = view;
}

void TextRenderer::execute() {
    std::unique_lock<std::mutex> lock(mMutex);
    if(mStop) {
        return;
    }

    // Check if current images has not been rendered, if not wait
    while(!mHasRendered) {
        mRenderedCV.wait(lock);
    }
    // This simply gets the input data for each connection and puts it into a data structure
    for(uint inputNr = 0; inputNr < getNrOfInputConnections(); inputNr++) {
        if(hasNewInputData(inputNr)) {
            DataObject::pointer input = getInputData<DataObject>(inputNr);

            mHasRendered = false;
            mDataToRender[inputNr] = input;
        }
    }
}

void TextRenderer::draw(Matrix4f perspectiveMatrix, Matrix4f viewingMatrix, bool mode2D) {
    std::lock_guard<std::mutex> lock(mMutex);

    if(!mode2D)
        throw Exception("TextRender is only implemented for 2D at the moment");

    if(mView == nullptr)
        throw Exception("TextRenderer need access to the view");

    for(auto it : mDataToRender) {
        Text::pointer input = std::static_pointer_cast<Text>(it.second);
        uint inputNr = it.first;

        // Check if a texture has already been created for this text
        if (mTexturesToRender.count(inputNr) > 0 && mTextUsed[inputNr] == input)
            continue; // If it has already been created, skip it

        if (mTexturesToRender.count(inputNr) > 0) {
            // Delete old texture
            glDeleteTextures(1, &mTexturesToRender[inputNr]);
            mTexturesToRender.erase(inputNr);
            glDeleteVertexArrays(1, &mVAO[inputNr]);
            mVAO.erase(inputNr);
        }

        Text::access access = input->getAccess(ACCESS_READ);
        std::string text = access->getData();


        // Font setup
        QColor color(mColor.getRedValue() * 255, mColor.getGreenValue() * 255, mColor.getBlueValue() * 255, 255);
        QFont font = QApplication::font();
        font.setPointSize(mFontSize);
        if (mStyle == STYLE_BOLD) {
            font.setBold(true);
        } else if (mStyle == STYLE_ITALIC) {
            font.setItalic(true);
        }
        QFontMetrics metrics(font);
        const int width = metrics.boundingRect(QString(text.c_str())).width() + 10;
        const int height = mFontSize + 5;

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
        std::cout << "Drawing at 0 " << width << " " << height << std::endl;
        uint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
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

        mTexturesToRender[inputNr] = textureID;
        mTextUsed[inputNr] = input;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    activateShader();
    for(auto it : mDataToRender) {
        const uint inputNr = it.first;
        Affine3f transform = Affine3f::Identity();

        // Get width and height of texture
        glBindTexture(GL_TEXTURE_2D, mTexturesToRender[inputNr]);
        int width, height;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
        const float scale = 1.0f / mView->width();
        const float scale2 = 1.0f / mView->height();
        const int padding = 15;
        Vector3f position;
        switch(mPosition) {
            case POSITION_CENTER:
                position = Vector3f(-width*scale/2.0f, -height*scale2/2.0f, 0); // Center
                break;
            case POSITION_TOP_CENTER:
                position = Vector3f(-width*scale/2.0f, 1 - (height + padding)*scale2, 0); // Top center
                break;
            case POSITION_BOTTOM_CENTER:
                position = Vector3f(-width*scale/2.0f, -1 + padding*scale2, 0); // Bottom center
                break;
            case POSITION_TOP_LEFT:
                position = Vector3f(-1 + padding*scale, 1 - (height + padding)*scale2, 0); // Top left corner
                break;
            case POSITION_TOP_RIGHT:
                position = Vector3f(1 - (width + padding)*scale, 1 - (height + padding)*scale2, 0); // Top right corner
                break;
            case POSITION_BOTTOM_LEFT:
                position = Vector3f(-1 + padding*scale, -1 + padding*scale2, 0); // Bottom left corner
                break;
            case POSITION_BOTTOM_RIGHT:
                position = Vector3f(1 - (width + padding)*scale, -1 + padding*scale2, 0); // Bottom right corner
                break;
        }
        transform.translate(position);
        transform.scale(Vector3f(scale, scale2, 0));

        uint transformLoc = glGetUniformLocation(getShaderProgram(), "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "perspectiveTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, perspectiveMatrix.data());
        transformLoc = glGetUniformLocation(getShaderProgram(), "viewTransform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, viewingMatrix.data());

        // Actual drawing
        glBindVertexArray(mVAO[inputNr]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindVertexArray(0);
    }
    glDisable(GL_BLEND);
    deactivateShader();
}

void TextRenderer::setFontSize(uint fontSize) {
	mFontSize = fontSize;
}

void TextRenderer::setPosition(TextRenderer::TextPosition position) {
    mPosition = position;
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

} // end namespace fast


