#include "View.hpp"
#include "Exception.hpp"
using namespace fast;

void View::addRenderer(Renderer::pointer renderer) {
    mRenderers->push_back(renderer);
}

View::View() {
}

void View::execute() {
}

void View::initializeGL() {
}

void View::paintGL() {
    bool success = glXMakeCurrent(XOpenDisplay(0),glXGetCurrentDrawable(),glContext);
    if(!success)
        throw Exception("failed to switch to window");

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for(unsigned int i = 0; i < mRenderers.size(); i++) {
        mRenderers[i]->draw();
    }


}

void View::resizeGL(int width, int height) {
}
