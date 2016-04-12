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
        std::vector<VectorXui> triangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mDimensions = 3;

    for(unsigned int i = 0; i < vertices.size(); i++) {
        MeshVertex v(vertices[i], normals[i]);
        mVertices.push_back(v);
    }

    for(unsigned int i = 0; i < triangles.size(); i++) {
        mVertices[triangles[i].x()].addConnection(i);
        mVertices[triangles[i].y()].addConnection(i);
        mVertices[triangles[i].z()].addConnection(i);
    }

    mBoundingBox = BoundingBox(vertices);
    mNrOfConnections = triangles.size();
    mConnections = triangles;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
    updateModifiedTimestamp();
}


void Mesh::create(std::vector<MeshVertex> vertices, std::vector<VectorXui> connections) {
     if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mDimensions = vertices[0].getNrOfDimensions();
    mVertices = vertices;
    std::vector<VectorXf> positions;
    for(unsigned int i = 0; i < vertices.size(); i++) {
    	VectorXf pos = vertices[i].getPosition();
        positions.push_back(pos);
    }
    mBoundingBox = BoundingBox(positions);
    mNrOfConnections = connections.size();
    mConnections = connections;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
}

void Mesh::create(unsigned int nrOfTriangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mNrOfConnections = nrOfTriangles;
}

VertexBufferObjectAccess::pointer Mesh::getVertexBufferObjectAccess(
        accessType type,
        OpenCLDevice::pointer device) {
    if(!mIsInitialized)
        throw Exception("Mesh has not been initialized.");

	if(mDimensions == 2)
		throw Exception("Not implemented for 2D");

    blockIfBeingWrittenTo();

    if (type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
    	{
            boost::unique_lock<boost::mutex> lock(mDataIsBeingWrittenToMutex);
            mDataIsBeingWrittenTo = true;
    	}
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
            reportInfo() << "created a drawable" << Reporter::end;
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
            float* data = new float[mNrOfConnections*18];
            for(uint i = 0; i < mNrOfConnections; i++) {
                Vector3ui triangle = mConnections[i];
                for(uint j = 0; j < 3; j++) {
                    MeshVertex vertex = mVertices[triangle[j]];
                    for(uint k = 0; k < 3; k++) {
                        data[counter+k] = vertex.getPosition()[k];
                        //reportInfo() << data[counter+k] << Reporter::end;
                        data[counter+3+k] = vertex.getNormal()[k];
                    }
                    //reportInfo() << "...." << Reporter::end;
                    counter += 6;
                }
            }
            glBufferData(GL_ARRAY_BUFFER, mNrOfConnections*18*sizeof(float), data, GL_STATIC_DRAW);
            delete[] data;
        } else {
            glBufferData(GL_ARRAY_BUFFER, mNrOfConnections*18*sizeof(float), NULL, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();
        if(glGetError() == GL_OUT_OF_MEMORY) {
        	throw Exception("OpenGL out of memory while creating mesh data for VBO");
        }
        //reportInfo() << "Created VBO with ID " << mVBOID << " and " << mNrOfConnections << " of triangles" << Reporter::end;
        // TODO Transfer data if any exist

        mVBOHasData = true;
        mVBODataIsUpToDate = true;

    } else {
        if(!mVBODataIsUpToDate) {
            // TODO Update data
        }
    }

    {
        boost::unique_lock<boost::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

	VertexBufferObjectAccess::pointer accessObject(new VertexBufferObjectAccess(mVBOID, mPtr.lock()));
	return std::move(accessObject);
}


// Hasher for the MeshVertex class
class KeyHasher {
    public:
        std::size_t operator()(const MeshVertex& v) const {
            using boost::hash_value;
            using boost::hash_combine;

            // Start with a hash value of 0    .
            std::size_t seed = 0;

            // Modify 'seed' by XORing and bit-shifting in
            // one member of 'Key' after the other:
            hash_combine(seed,hash_value(v.getPosition()[0]));
            hash_combine(seed,hash_value(v.getPosition()[1]));
            hash_combine(seed,hash_value(v.getPosition()[2]));

            // Return the result.
            return seed;
        }
};

bool operator==(const MeshVertex& a, const MeshVertex& b) {
    return a.getPosition()[0] == b.getPosition()[0] &&
    a.getPosition()[1] == b.getPosition()[1] &&
    a.getPosition()[2] == b.getPosition()[2];
}

MeshAccess::pointer Mesh::getMeshAccess(accessType type) {
    if(!mIsInitialized) {
        throw Exception("Surface has not been initialized.");
    }

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
    	{
    		boost::lock_guard<boost::mutex> lock(mDataIsBeingWrittenToMutex);
            mDataIsBeingWrittenTo = true;
    	}
        updateModifiedTimestamp();
    }
    if(!mHostHasData) {
    	if(mDimensions == 2)
    		throw Exception("Not implemented for 2D");
        // Get all vertices with normals from VBO (including duplicates)
        float* data = new float[mNrOfConnections*18];
        glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*mNrOfConnections*18, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        std::vector<MeshVertex> vertices;
        std::vector<VectorXui> triangles;
        boost::unordered_map<MeshVertex, uint, KeyHasher> vertexList;
        for(int t = 0; t < mNrOfConnections; t++) {
            Vector3ui triangle;
            for(int v = 0; v < 3; v++) {
            	Vector3f position;
            	Vector3f normal;
                position[0] = data[t*18+v*6];
                position[1] = data[t*18+v*6+1];
                position[2] = data[t*18+v*6+2];
                normal[0] = data[t*18+v*6+3];
                normal[1] = data[t*18+v*6+4];
                normal[2] = data[t*18+v*6+5];
                MeshVertex vertex(position, normal);
                vertex.addConnection(t);

                // Only add if not a duplicate
                if(vertexList.count(vertex) > 0) {
                    // Found a duplicate
                    // Get index of duplicate
                    const uint duplicateIndex = vertexList[vertex];
                    // Add this triangle to the duplicate
                    MeshVertex& duplicate = vertices[duplicateIndex];
                    duplicate.addConnection(t);
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
        mConnections = triangles;
        mVertices = vertices;
        mHostHasData = true;
        mHostDataIsUpToDate = true;
        delete[] data;
    } else {
        if(!mHostDataIsUpToDate) {
            throw Exception("Not implemented yet!");
        }
    }

    {
        boost::lock_guard<boost::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

    MeshAccess::pointer accessObject(new MeshAccess(&mVertices,&mConnections,mPtr.lock()));
	return std::move(accessObject);
}

Mesh::~Mesh() {
    freeAll();
}

Mesh::Mesh() {
    mIsInitialized = false;
    mVBOHasData = false;
    mHostHasData = false;
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
    return mNrOfConnections;
}

unsigned int Mesh::getNrOfVertices() const {
    return mVertices.size();
}

void Mesh::setBoundingBox(BoundingBox box) {
    mBoundingBox = box;
}

void Mesh::create(std::vector<Vector2f> vertices, std::vector<Vector2f> normals,
		std::vector<VectorXui> lines) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mIsInitialized = true;
    mDimensions = 2;

    for(unsigned int i = 0; i < vertices.size(); i++) {
        MeshVertex v(vertices[i], normals[i]);
        mVertices.push_back(v);
    }

    for(unsigned int i = 0; i < lines.size(); i++) {
        mVertices[lines[i].x()].addConnection(i);
        mVertices[lines[i].y()].addConnection(i);
    }

    mBoundingBox = BoundingBox(vertices);
    mNrOfConnections = lines.size();
    mConnections = lines;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
    updateModifiedTimestamp();
}

unsigned int Mesh::getNrOfLines() const {
	return mNrOfConnections;
}

uchar Mesh::getDimensions() const {
	return mDimensions;
}

} // end namespace fast


