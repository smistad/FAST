
#include "TextRenderer.hpp"
#include "FAST/Visualization/View.hpp"
#include <QPainter>

namespace fast {


BoundingBox TextRenderer::getBoundingBox() {
    return BoundingBox(Vector3f(6,6,6));
}

TextRenderer::TextRenderer() {
    createInputPort<Text>(0);
    mStyle = STYLE_NORMAL;
	mFontSize = 18;
	mPosition2D = Vector2i::Zero();
	mColor = Color::Green();
    mText = "";
    createIntegerAttribute("position", "Text position", "Position of text in view", 0);
    createIntegerAttribute("font_size", "Font size", "Font size", mFontSize);
}

void TextRenderer::setView(View* view) {
	mView = view;
}

void TextRenderer::execute() {
    std::lock_guard<std::mutex> lock(mMutex);

    Text::pointer text = getStaticInputData<Text>();
    Text::access access = text->getAccess(ACCESS_READ);
    mText = access->getData();
}

void TextRenderer::draw() {
    std::lock_guard<std::mutex> lock(mMutex);

    if(mView != NULL) {
        if(mText != "") {
            Color color = mColor;
            QFont font;
            font.setPointSize(mFontSize);
            if (mStyle == STYLE_BOLD) {
                font.setBold(true);
            } else if (mStyle == STYLE_ITALIC) {
                font.setItalic(true);
            }
            glColor3f(color.getRedValue(), color.getGreenValue(), color.getBlueValue());
            mView->renderText(mPosition2D.x(), mPosition2D.y(), mText.c_str(), font);
            glColor3f(1.0f, 1.0f, 1.0f);
        }
    } else {
    	reportWarning() << "View must be given to text render in order for it to work." << reportEnd();
    }
}

void TextRenderer::setPosition(Vector2i position) {
	mPosition2D = position;
}

void TextRenderer::setFontSize(uint fontSize) {
	mFontSize = fontSize;
}

void TextRenderer::setColor(Color color) {
	mColor = color;
}

void TextRenderer::setStyle(TextStyleType textStyleType) {
	mStyle = textStyleType;
}

void TextRenderer::draw2D(cl::Buffer PBO, uint width, uint height,
                          Eigen::Transform<float, 3, Eigen::Affine> pixelToViewportTransform, float PBOspacing,
                          Vector2f translation) {
    std::lock_guard<std::mutex> lock(mMutex);

    if(mPosition2D == Vector2i::Zero()) {
        // If no position has been set. Set it to upper left corner
        mPosition2D = Vector2i(10, mFontSize + 10);
    }

    // TODO select a safe background color here, that is different from set color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (mView != NULL) {
        if (mText != "") {
            Color color = mColor;
            QFont font;
            font.setPointSize(mFontSize);
            if (mStyle == STYLE_BOLD) {
                font.setBold(true);
            } else if (mStyle == STYLE_ITALIC) {
                font.setItalic(true);
            }
            // Render text
            /*
            QColor fontColor = QColor(round(color.getRedValue()*255), round(color.getGreenValue()*255), round(color.getBlueValue()*255));
            QPainter painter(mView);
            painter.setPen(fontColor);
            painter.setFont(font);
            painter.drawText(mPosition2D.x(), mPosition2D.y(), mText.c_str());
            painter.end();
             */
            glColor3f(color.getRedValue(), color.getGreenValue(), color.getBlueValue());
            mView->renderText(mPosition2D.x(), mPosition2D.y(), mText.c_str(), font);
            glColor3f(1.0f, 1.0f, 1.0f);
        }
    } else {
        reportWarning() << "View must be given to text render in order for it to work." << reportEnd();
    }

    //mView->swapBuffers();
    QImage image = mView->grabFrameBuffer();
    // TODO get clPBO data
    OpenCLDevice::pointer device = getMainDevice();
    cl::CommandQueue queue = device->getCommandQueue();
    float* data = new float[width*height*4];
    queue.enqueueReadBuffer(
        PBO,
        CL_TRUE,
        0,
        sizeof(float)*width*height*4,
        data
    );

    for (int y = 0; y < height; ++y) {
        QRgb *line = (QRgb *) image.scanLine(y);
        for (int x = 0; x < width; ++x) {
            QRgb* pixel = line + x;
            QColor color(*pixel);
            // Only write, if not color is not background color
            if(color.green() > 0.001f || color.red() > 0.001f || color.blue() > 0.001f) {
                data[(x + (height - 1 - y) * width) * 4] = (float) color.red() / 255.0f;
                data[(x + (height - 1 - y) * width) * 4 + 1] = (float) color.green() / 255.0f;
                data[(x + (height - 1 - y) * width) * 4 + 2] = (float) color.blue() / 255.0f;
                data[(x + (height - 1 - y) * width) * 4 + 3] = 1.0f;
            }
        }
    }

    // TODO Transfer data back to clPBO
    queue.enqueueWriteBuffer(
            PBO,
            CL_TRUE,
            0,
            sizeof(float)*width*height*4,
            data
    );

    delete[] data;

    // TODO clear buffer glClear
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
}

void TextRenderer::loadAttributes() {
    std::vector<int> position = getIntegerListAttribute("position");
    if(position.size() == 2)
        setPosition(Vector2i(position.at(0), position.at(1)));
    setFontSize(getIntegerAttribute("font_size"));
}

} // end namespace fast


