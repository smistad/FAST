#include "Object.hpp"
#include <iostream>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/glx.h>
#endif
#endif

namespace fast {

void Object::setOpenGLContext(unsigned long* OpenGLContext) {
    boost::unique_lock<boost::mutex> lock(Object::GLmutex);
    while(!Object::GLcontextReady)
    {
        Object::condition.wait(lock);
    }
    std::cout << "acquired access to GL" << std::endl;
    Object::GLcontextReady = false;
#if defined(__APPLE__) || defined(__MACOSX)
    // Returns 0 on success
    //bool success = CGLSetCurrentContext((CGLContextObj)OpenGLContext) == 0;
	// Do nothing for apple
	bool success = true;
#else
#if _WIN32
    bool success = wglMakeCurrent(wglGetCurrentDC(), (HGLRC)OpenGLContext);
#else
    static Display * mXDisplay = XOpenDisplay(NULL);
    std::cout << "drawable: " << currentDrawable << std::endl;
    bool success = glXMakeCurrent(mXDisplay,currentDrawable,(GLXContext)OpenGLContext);
    //std::cout << "drawable: " << glXGetCurrentDrawable() << std::endl;
    //bool success = glXMakeCurrent(mXDisplay,glXGetCurrentDrawable(),(GLXContext)OpenGLContext);
#endif
#endif
    if(!success)
        throw Exception("Failed to make the OpenGL Context current");

}

void Object::releaseOpenGLContext() {
    std::cout << "trying to release GL.." << std::endl;
    static Display * mXDisplay = XOpenDisplay(NULL);
    {
        boost::lock_guard<boost::mutex> lock(Object::GLmutex);
        std::cout << "releasing GL" << std::endl;
        glXMakeCurrent(mXDisplay, None, NULL);
        Object::GLcontextReady = true;
    }
    Object::condition.notify_one();
}

unsigned long Object::currentDrawable;
boost::condition_variable Object::condition;
bool Object::GLcontextReady = true;
boost::mutex Object::GLmutex;

} // end namespace fast
