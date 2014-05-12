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
    // Set up a timer that will call update on this object at a regular interval
    timer = new QTimer(this);
    timer->start(50); // in milliseconds
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(update()));
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
}

void View::resizeGL(int width, int height) {
}

void View::keyPressEvent(QKeyEvent* event) {
    std::cout << "keyboard used" << std::endl;
    // Relay keyboard event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->keyPressEvent(event);
    }
}

void View::mouseMoveEvent(QMouseEvent* event) {
    std::cout << "mouse moved" << std::endl;
    // Relay mouse event info to renderers
    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->mouseMoveEvent(event);
    }
}

void View::mousePressEvent(QMouseEvent* event) {
    std::cout << "mouse used" << std::endl;
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
