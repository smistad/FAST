#include "View.hpp"
#include "Exception.hpp"

#include <GL/glx.h>

using namespace fast;

void View::addRenderer(Renderer::pointer renderer) {
    mRenderers.push_back(renderer);
}

View::View() {
    timer = new QTimer(this);
    timer->start(100); // in milliseconds
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
        mRenderers[i]->update(); // TODO should not run update so often..
        mRenderers[i]->draw();
    }
}

void View::resizeGL(int width, int height) {
}
