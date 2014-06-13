#include <GL/glew.h>
#include "Surface.hpp"
#include "SimpleWindow.hpp"
#include <QApplication>
#include <GL/glx.h>

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
        if(glXGetCurrentDrawable() == 0) { // TODO make this work on all platforms
            SimpleWindow::initializeQtApp();

            // Need a drawable for this to work
            QGLWidget* widget = new QGLWidget;
            widget->show();

            widget->hide(); // TODO should probably delete widget as well
            std::cout << "created a drawable" << std::endl;
        }
        setOpenGLContext(device->getGLContext());
        GLenum err = glewInit();
        if(err != GLEW_OK)
            throw Exception("GLEW init error");
        glGenBuffers(1, &mVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        glBufferData(GL_ARRAY_BUFFER, mNrOfTriangles*18*sizeof(cl_float), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();
        std::cout << "Created VBO with ID " << mVBOID << std::endl;
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

BoundingBox Surface::getBoundingBox() const {
    return mBoundingBox;
}

void Surface::setBoundingBox(BoundingBox box) {
    mBoundingBox = box;
}

} // end namespace fast

