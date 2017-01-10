
#include "TextRenderer.hpp"
#include "FAST/Visualization/View.hpp"

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
            std::cout << "rendering text " << mText << std::endl;
            Color color = mColor;
            glColor3f(color.getRedValue(), color.getGreenValue(), color.getBlueValue());
            QFont font;
            font.setPointSize(mFontSize);
            if (mStyle == STYLE_BOLD) {
                font.setBold(true);
            } else if (mStyle == STYLE_ITALIC) {
                font.setItalic(true);
            }
            mView->renderText((int) mPosition2D.x(), (int) mPosition2D.y(), mText.c_str(), font);
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

    // TODO select a safe background color here, that is different from set color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (mView != NULL) {
        if (mText != "") {
            std::cout << "rendering text " << mText << std::endl;
            Color color = mColor;
            glColor4f(color.getRedValue(), color.getGreenValue(), color.getBlueValue(), 1.0f);
            QFont font;
            font.setPointSize(mFontSize);
            if (mStyle == STYLE_BOLD) {
                font.setBold(true);
            } else if (mStyle == STYLE_ITALIC) {
                font.setItalic(true);
            }
            mView->renderText((int) mPosition2D.x(), (int) mPosition2D.y(), mText.c_str(), font);
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
                data[(x + (width - 1 - y) * width) * 4] = (float) color.red() / 255.0f;
                data[(x + (width - 1 - y) * width) * 4 + 1] = (float) color.green() / 255.0f;
                data[(x + (width - 1 - y) * width) * 4 + 2] = (float) color.blue() / 255.0f;
                data[(x + (width - 1 - y) * width) * 4 + 3] = 1.0f;
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

} // end namespace fast


