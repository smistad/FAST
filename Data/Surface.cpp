#include <GL/glew.h>
#include <GL/glx.h>
#include "Surface.hpp"

namespace fast {

void Surface::create(
        std::vector<Float<3> > vertices,
        std::vector<Float<3> > normals,
        std::vector<Uint<3> > triangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;

    for(unsigned int i = 0; i < vertices.size(); i++) {
        SurfaceVertex v;
        v.position = vertices[i];
        v.normal = normals[i];
        mVertices.push_back(v);
    }

    for(unsigned int i = 0; i < triangles.size(); i++) {
        mVertices[triangles[i].x()].triangles.push_back(i);
        mVertices[triangles[i].y()].triangles.push_back(i);
        mVertices[triangles[i].z()].triangles.push_back(i);
    }

    mNrOfTriangles = triangles.size();
}

void Surface::create(unsigned int nrOfTriangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mNrOfTriangles = nrOfTriangles;
}

bool Surface::isAnyDataBeingAccessed() {
    return mVBODataIsBeingAccessed || mHostDataIsBeingAccessed;
}

VertexBufferObjectAccess Surface::getVertexBufferObjectAccess(
        accessType type,
        OpenCLDevice::pointer device) {
    if(!mIsInitialized)
        throw Exception("Surface has not been initialized.");

    if(mSurfaceIsBeingWrittenTo)
        throw Exception("Requesting access to an image that is already being written to.");
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mSurfaceIsBeingWrittenTo = true;
    }
    if(!mVBOHasData) {
        // TODO create VBO
        // Have to have a drawable available before glewInit and glGenBuffers
        if(glXGetCurrentDrawable() == 0) {
            int sngBuf[] = { GLX_RGBA,
                             GLX_DOUBLEBUFFER,
                             GLX_RED_SIZE, 1,
                             GLX_GREEN_SIZE, 1,
                             GLX_BLUE_SIZE, 1,
                             GLX_DEPTH_SIZE, 12,
                             None
            };
            Display * display = XOpenDisplay(NULL);
            //Window w = XCreateSimpleWindow(display,NULL,0,0,256,256,0,0,0);
            XWindowAttributes attribs;
            XGetWindowAttributes(display, XDefaultRootWindow(display), &attribs);
            Pixmap pixmap = XCreatePixmap(display,XDefaultRootWindow(display),256,256,attribs.depth);
            XVisualInfo* vi = glXChooseVisual(display, DefaultScreen(display), sngBuf);
            GLXPixmap glxPixmap = glXCreateGLXPixmap(display, vi, pixmap);
            bool success = glXMakeCurrent(XOpenDisplay(NULL),glxPixmap,(GLXContext)device->getGLContext());
            std::cout << "Current drawable: " << glXGetCurrentDrawable() << std::endl;
            std::cout << device->getGLContext() << std::endl;
            if(!success)
                std::cout << "iiik" << std::endl;
        }
        GLenum err = glewInit();
        if(err != GLEW_OK)
            std::cout << "GLEW error" << std::endl;
        glGenBuffers(1, &mVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        glBufferData(GL_ARRAY_BUFFER, mNrOfTriangles*18*sizeof(cl_float), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();

        // TODO Transfer data if any exist
        mVBOHasData = true;
        mVBODataIsUpToDate = true;

    } else {
        if(!mVBODataIsUpToDate) {
            // TODO Update data
        }
    }

    mVBODataIsBeingAccessed = true;

    return VertexBufferObjectAccess(mVBOID, &mVBODataIsBeingAccessed, &mSurfaceIsBeingWrittenTo);
}

SurfacePointerAccess Surface::getSurfacePointerAccess(accessType access) {
    if(!mIsInitialized)
        throw Exception("Surface has not been initialized.");

    throw Exception("Not implemented yet!");
}

Surface::~Surface() {
    freeAll();
}

Surface::Surface() {
    mIsInitialized = false;
    mVBOHasData = false;
    mHostHasData = false;
    mSurfaceIsBeingWrittenTo = false;
    mVBODataIsBeingAccessed = false;
    mHostDataIsBeingAccessed = false;
}

void Surface::freeAll() {
    // TODO finish
    if(mVBOHasData)
        glDeleteBuffers(1, &mVBOID);
}

void Surface::free(ExecutionDevice::pointer device) {
    // TODO
}

unsigned int Surface::getNrOfTriangles() const {
    return mNrOfTriangles;
}

} // end namespace fast


