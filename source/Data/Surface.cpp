#include <GL/glew.h>
#include "Surface.hpp"
#include "SimpleWindow.hpp"
#include "SceneGraph.hpp"
#include <QApplication>

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

void Surface::create(
        std::vector<Float3> vertices,
        std::vector<Float3> normals,
        std::vector<Uint3> triangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;

    for(unsigned int i = 0; i < vertices.size(); i++) {
        SurfaceVertex v;
        v.position(0) = vertices[i].x();
        v.position(1) = vertices[i].y();
        v.position(2) = vertices[i].z();
        v.normal(0) = normals[i].x();
        v.normal(1) = normals[i].y();
        v.normal(2) = normals[i].z();
        mVertices.push_back(v);
    }

    for(unsigned int i = 0; i < triangles.size(); i++) {
        mVertices[triangles[i].x()].triangles.push_back(i);
        mVertices[triangles[i].y()].triangles.push_back(i);
        mVertices[triangles[i].z()].triangles.push_back(i);
    }

    mBoundingBox = BoundingBox(vertices);
    mNrOfTriangles = triangles.size();
    mTriangles = triangles;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
    SceneGraph& graph = SceneGraph::getInstance();
    graph.addDataNodeToNewRoot(mPtr);
}


void Surface::create(std::vector<SurfaceVertex> vertices, std::vector<Uint3> triangles) {
     if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mVertices = vertices;
    std::vector<Float3> positions;
    for(unsigned int i = 0; i < vertices.size(); i++) {
        Float3 pos(vertices[i].position(0), vertices[i].position(1), vertices[i].position(2));
        positions.push_back(pos);
    }
    mBoundingBox = BoundingBox(positions);
    mNrOfTriangles = triangles.size();
    mTriangles = triangles;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
    SceneGraph& graph = SceneGraph::getInstance();
    graph.addDataNodeToNewRoot(mPtr);
}

void Surface::create(unsigned int nrOfTriangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mNrOfTriangles = nrOfTriangles;
    SceneGraph& graph = SceneGraph::getInstance();
    graph.addDataNodeToNewRoot(mPtr);
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
        throw Exception("Requesting access to a surface that is already being written to.");
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
#if defined(__APPLE__) || defined(__MACOSX)
#else
#if _WIN32
#else
        // If no Window is present, create a dummy gl context
        if(!QApplication::instance()) { // TODO make this work on all platforms
            SimpleWindow::initializeQtApp();

            // Need a drawable for this to work
            QGLWidget* widget = new QGLWidget;
            widget->show();

            widget->hide(); // TODO should probably delete widget as well
            std::cout << "created a drawable" << std::endl;
        }
#endif
#endif
        setOpenGLContext(device->getGLContext());
        GLenum err = glewInit();
        if(err != GLEW_OK)
            throw Exception("GLEW init error");
        glGenBuffers(1, &mVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        if(mHostHasData) {
            // If host has data, transfer it.
            // Create data arrays with vertices and normals interleaved
            uint counter = 0;
            float* data = new float[mNrOfTriangles*18];
            for(uint i = 0; i < mNrOfTriangles; i++) {
                Uint3 triangle = mTriangles[i];
                for(uint j = 0; j < 3; j++) {
                    SurfaceVertex vertex = mVertices[triangle[j]];
                    for(uint k = 0; k < 3; k++) {
                        data[counter+k] = vertex.position[k];
                        //std::cout << data[counter+k] << std::endl;
                        data[counter+3+k] = vertex.normal[k];
                    }
                    //std::cout << "...." << std::endl;
                    counter += 6;
                }
            }
            glBufferData(GL_ARRAY_BUFFER, mNrOfTriangles*18*sizeof(float), data, GL_STATIC_DRAW);
            delete[] data;
        } else {
            glBufferData(GL_ARRAY_BUFFER, mNrOfTriangles*18*sizeof(float), NULL, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();
        releaseOpenGLContext();
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

SurfacePointerAccess Surface::getSurfacePointerAccess(accessType type) {
    if(!mIsInitialized) {
        this->update();
    }
    if(mSurfaceIsBeingWrittenTo)
        throw Exception("Requesting access to a surface that is already being written to.");
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mSurfaceIsBeingWrittenTo = true;
    }
    if(!mHostHasData) {
        throw Exception("Not implemented yet!");
    } else {
        if(!mHostDataIsUpToDate) {
            throw Exception("Not implemented yet!");
        }
    }

    mHostDataIsBeingAccessed = true;
    return SurfacePointerAccess(&mVertices,&mTriangles,&mHostDataIsBeingAccessed,&mSurfaceIsBeingWrittenTo);
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
    if(mVBOHasData) {
        glDeleteBuffers(1, &mVBOID);
    }
    mVBOHasData = false;
}

void Surface::free(ExecutionDevice::pointer device) {
    // TODO
}

unsigned int Surface::getNrOfTriangles() const {
    return mNrOfTriangles;
}

unsigned int Surface::getNrOfVertices() const {
    return mVertices.size();
}

void Surface::setBoundingBox(BoundingBox box) {
    mBoundingBox = box;
}

} // end namespace fast

