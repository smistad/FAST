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
    boost::unique_lock<boost::mutex> lock(Object::GLmutex); // this locks the mutex
    while(!Object::GLcontextReady)
    {
        // Unlocks the mutex and wait until someone calls notify.
        // When it wakes, the mutex is locked again and GLcontextReady is checked.
        Object::condition.wait(lock);
    }
    Object::GLcontextReady = false;
#if defined(__APPLE__) || defined(__MACOSX)
    // Returns 0 on success
    //bool success = CGLSetCurrentContext((CGLContextObj)(OpenGLContext)) == 0;
    //Object::mGLContext->makeCurrent();
    //std::cout << "Context: " << (CGLContextObj)OpenGLContext << std::endl;
    //std::cout << "Share group 1: " << CGLGetShareGroup((CGLContextObj)(OpenGLContext)) << std::endl;
    //std::cout << "Share group 2: " << CGLGetShareGroup((CGLContextObj)(currentDrawable)) << std::endl;
    //std::cout << "Share group 3: " << CGLGetShareGroup(CGLGetCurrentContext()) << std::endl;
	// Do nothing for apple
	bool success = true;
#else
#if _WIN32
	HDC hdc = (HDC)Object::hdc;
    bool success = wglMakeCurrent(hdc, (HGLRC)OpenGLContext);
#else
    static Display * mXDisplay = XOpenDisplay(NULL);
    //std::cout << "drawable: " << currentDrawable << std::endl;
    bool success = glXMakeCurrent(mXDisplay,currentDrawable,(GLXContext)OpenGLContext);
    //std::cout << "drawable: " << glXGetCurrentDrawable() << std::endl;
    //bool success = glXMakeCurrent(mXDisplay,glXGetCurrentDrawable(),(GLXContext)OpenGLContext);
#endif
#endif
    if(!success)
        throw Exception("Failed to make the OpenGL Context current");

}

void Object::releaseOpenGLContext() {
    {
        boost::lock_guard<boost::mutex> lock(Object::GLmutex); // lock mutex
#if defined(__APPLE__) || defined(__MACOSX)
        // Mac TODO verify that this works
        //bool success = CGLSetCurrentContext(NULL) == 0;
        // Do nothing for apple
        bool success = true;
#elif _WIN32
        // Windows
		HDC hdc = (HDC)Object::hdc;
        bool success = wglMakeCurrent(hdc, NULL);
#else
        // Linux
        static Display * mXDisplay = XOpenDisplay(NULL);
        bool success = glXMakeCurrent(mXDisplay, None, NULL);
#endif
        if(!success)
            throw Exception("Failed to release OpenGL Context");
        Object::GLcontextReady = true;
    }
    // Notify other waiting threads that the GL context is released
    Object::condition.notify_one();
}

unsigned long Object::currentDrawable;
void* Object::hdc;
boost::condition_variable Object::condition;
bool Object::GLcontextReady = true;
boost::mutex Object::GLmutex;

} // end namespace fast
