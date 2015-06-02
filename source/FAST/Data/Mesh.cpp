#include <GL/glew.h>
#include "Mesh.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"
#include <QApplication>
#include <boost/thread.hpp>

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

void Mesh::create(
        std::vector<Vector3f> vertices,
        std::vector<Vector3f> normals,
        std::vector<Vector3ui> triangles) {
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
    updateModifiedTimestamp();
}


void Mesh::create(std::vector<SurfaceVertex> vertices, std::vector<Vector3ui> triangles) {
     if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mVertices = vertices;
    std::vector<Vector3f> positions;
    for(unsigned int i = 0; i < vertices.size(); i++) {
        positions.push_back(vertices[i].position);
    }
    mBoundingBox = BoundingBox(positions);
    mNrOfTriangles = triangles.size();
    mTriangles = triangles;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
}

void Mesh::create(unsigned int nrOfTriangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mNrOfTriangles = nrOfTriangles;
}

bool Mesh::isAnyDataBeingAccessed() {
    return mVBODataIsBeingAccessed || mHostDataIsBeingAccessed;
}

VertexBufferObjectAccess::pointer Mesh::getVertexBufferObjectAccess(
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
        updateModifiedTimestamp();
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
                Vector3ui triangle = mTriangles[i];
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
        //std::cout << "Created VBO with ID " << mVBOID << " and " << mNrOfTriangles << " of triangles" << std::endl;
        // TODO Transfer data if any exist

        mVBOHasData = true;
        mVBODataIsUpToDate = true;

    } else {
        if(!mVBODataIsUpToDate) {
            // TODO Update data
        }
    }

    mVBODataIsBeingAccessed = true;

	VertexBufferObjectAccess::pointer accessObject(new VertexBufferObjectAccess(mVBOID, &mVBODataIsBeingAccessed, &mSurfaceIsBeingWrittenTo));
	return std::move(accessObject);
}


// Hasher for the SurfaceVertex class
class KeyHasher {
    public:
        std::size_t operator()(const SurfaceVertex& v) const {
            using boost::hash_value;
            using boost::hash_combine;

            // Start with a hash value of 0    .
            std::size_t seed = 0;

            // Modify 'seed' by XORing and bit-shifting in
            // one member of 'Key' after the other:
            hash_combine(seed,hash_value(v.position[0]));
            hash_combine(seed,hash_value(v.position[1]));
            hash_combine(seed,hash_value(v.position[2]));

            // Return the result.
            return seed;
        }
};

bool operator==(const SurfaceVertex& a, const SurfaceVertex& b) {
    return a.position[0] == b.position[0] &&
    a.position[1] == b.position[1] &&
    a.position[2] == b.position[2];
}

SurfacePointerAccess::pointer Mesh::getSurfacePointerAccess(accessType type) {
    if(!mIsInitialized) {
        throw Exception("Surface has not been initialized.");
    }
    if(mSurfaceIsBeingWrittenTo)
        throw Exception("Requesting access to a surface that is already being written to.");
    if (type == ACCESS_READ_WRITE) {
        if (isAnyDataBeingAccessed()) {
            throw Exception(
                    "Trying to get write access to an object that is already being accessed");
        }
        mSurfaceIsBeingWrittenTo = true;
        updateModifiedTimestamp();
    }
    if(!mHostHasData) {
        // Get all vertices with normals from VBO (including duplicates)
        float* data = new float[mNrOfTriangles*18];
        glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*mNrOfTriangles*18, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        std::vector<SurfaceVertex> vertices;
        std::vector<Vector3ui> triangles;
        boost::unordered_map<SurfaceVertex, uint, KeyHasher> vertexList;
        for(int t = 0; t < mNrOfTriangles; t++) {
            Vector3ui triangle;
            for(int v = 0; v < 3; v++) {
                SurfaceVertex vertex;
                vertex.position[0] = data[t*18+v*6];
                vertex.position[1] = data[t*18+v*6+1];
                vertex.position[2] = data[t*18+v*6+2];
                vertex.normal[0] = data[t*18+v*6+3];
                vertex.normal[1] = data[t*18+v*6+4];
                vertex.normal[2] = data[t*18+v*6+5];
                vertex.triangles.push_back(t);

                // Only add if not a duplicate
                if(vertexList.count(vertex) > 0) {
                    // Found a duplicate
                    // Get index of duplicate
                    const uint duplicateIndex = vertexList[vertex];
                    // Add this triangle to the duplicate
                    SurfaceVertex& duplicate = vertices[duplicateIndex];
                    duplicate.triangles.push_back(t);
                    // Add the vertex to this triangle
                    triangle[v] = duplicateIndex;
                } else {
                    // If duplicate was not found, add it to the list
                    vertices.push_back(vertex);
                    triangle[v] = vertices.size()-1;
                    vertexList[vertex] = vertices.size()-1;
                }
            }
            triangles.push_back(triangle);
        }
        mTriangles = triangles;
        mVertices = vertices;
        mHostHasData = true;
        mHostDataIsUpToDate = true;
        delete[] data;
    } else {
        if(!mHostDataIsUpToDate) {
            throw Exception("Not implemented yet!");
        }
    }

    mHostDataIsBeingAccessed = true;
    SurfacePointerAccess::pointer accessObject(new SurfacePointerAccess(&mVertices,&mTriangles,&mHostDataIsBeingAccessed,&mSurfaceIsBeingWrittenTo));
	return std::move(accessObject);
}

Mesh::~Mesh() {
    freeAll();
}

Mesh::Mesh() {
    mIsInitialized = false;
    mVBOHasData = false;
    mHostHasData = false;
    mSurfaceIsBeingWrittenTo = false;
    mVBODataIsBeingAccessed = false;
    mHostDataIsBeingAccessed = false;
}

void Mesh::freeAll() {
    // TODO finish
    if(mVBOHasData) {
        // glDeleteBuffer is not used due to multi-threading issues..
        //glDeleteBuffers(1, &mVBOID);
        glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        // This should delete the data, by replacing it with 1 byte buffer
        // Ideally it should be 0, but then the data is not deleted..
        glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    mVBOHasData = false;
}

void Mesh::free(ExecutionDevice::pointer device) {
    // TODO
}

unsigned int Mesh::getNrOfTriangles() const {
    return mNrOfTriangles;
}

unsigned int Mesh::getNrOfVertices() const {
    return mVertices.size();
}

void Mesh::setBoundingBox(BoundingBox box) {
    mBoundingBox = box;
}

} // end namespace fast

