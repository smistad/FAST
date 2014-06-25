#include "Object.hpp"

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
    bool success = glXMakeCurrent(mXDisplay,glXGetCurrentDrawable(),(GLXContext)OpenGLContext);
#endif
#endif
    if(!success)
        throw Exception("Failed to make the OpenGL Context current");
}

} // end namespace fast
