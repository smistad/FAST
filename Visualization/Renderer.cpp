#include "Renderer.hpp"
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <GL/gl.h>
#else
#include <GL/glx.h>
#endif
#endif
using namespace fast;

#if defined(__APPLE__) || defined(__MACOSX)
#else
#if _WIN32
#else
static Display * mXDisplay;
#endif
#endif

Renderer::Renderer() {
    mWindow = -1;
    mLevel = -1;
#if defined(__APPLE__) || defined(__MACOSX)
#else
#if _WIN32
#else
    // Open the display here to avoid getting maximum number of clients error
    mXDisplay = XOpenDisplay(NULL);
#endif
#endif
}

void Renderer::setIntensityLevel(float level) {
    mLevel = level;
}

float Renderer::getIntensityLevel() {
    return mLevel;
}

void Renderer::setIntensityWindow(float window) {
    if(window <= 0)
        throw Exception("Intensity window has to be above 0.");
    mWindow = window;
}

float Renderer::getIntensityWindow() {
    return mWindow;
}


void Renderer::setOpenGLContext(unsigned long* OpenGLContext) {
#if defined(__APPLE__) || defined(__MACOSX)
    // Returns 0 on success
    //bool success = CGLSetCurrentContext((CGLContextObj)OpenGLContext) == 0;
	// Do nothing for apple
	bool success = true;
#else
#if _WIN32
    bool success = wglMakeCurrent(wglGetCurrentDC(), (HGLRC)OpenGLContext);
#else
    bool success = glXMakeCurrent(mXDisplay,glXGetCurrentDrawable(),(GLXContext)OpenGLContext);
#endif
#endif
    if(!success)
        throw Exception("Failed to make the OpenGL Context current");
}
