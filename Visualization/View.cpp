#include "View.hpp"
#include "Exception.hpp"

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl_gl.h>
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#include <CL/cl_gl.h>
#else
#include <GL/glx.h>
#include <CL/cl_gl.h>
#endif
#endif

using namespace fast;

void View::addRenderer(Renderer::pointer renderer) {
    mRenderers.push_back(renderer);
}

View::View() {
    mFramerate = 25;
    // Set up a timer that will call update on this object at a regular interval
    timer = new QTimer(this);
    timer->start(1000/mFramerate); // in milliseconds
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
}

void View::setMaximumFramerate(unsigned int framerate) {
    if(framerate == 0)
        throw Exception("Framerate cannot be 0.");

    mFramerate = framerate;
    timer->stop();
    timer->start(1000/mFramerate); // in milliseconds
    timer->setSingleShot(false);
}

void View::execute() {
}

void View::initializeGL() {
}

void View::paintGL() {

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->update();
        mRenderers[i]->draw();
    }
    //swapBuffers();
}

void View::resizeGL(int width, int height) {
}

void View::keyPressEvent(QKeyEvent* event) {
    // Relay keyboard event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->keyPressEvent(event);
    }
}

void View::mouseMoveEvent(QMouseEvent* event) {
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->mouseMoveEvent(event, this);
    }
}

void View::mousePressEvent(QMouseEvent* event) {
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->mousePressEvent(event);
    }
}

void View::mouseReleaseEvent(QMouseEvent* event) {
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->mouseReleaseEvent(event);
    }
}
void View::resizeEvent(QResizeEvent* event) {
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->resizeEvent(event);
    }
}
