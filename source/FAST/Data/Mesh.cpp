#include "Mesh.hpp"
#include <thread>
#include "FAST/Utility.hpp"

#ifdef FAST_MODULE_VISUALIZATION
#include "FAST/Visualization/Window.hpp"
#include <QApplication>
#include <QOpenGLFunctions_3_3_Core>
#endif


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
        std::vector<Vector3f> normals
        ) {
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

    mBoundingBox = BoundingBox(vertices);
    mNrOfConnections = 0;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
    updateModifiedTimestamp();
}

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
    if(vertices.size() == 0) {
    	create(0);
    	return;
    }

    mIsInitialized = true;
    mVertices = vertices;
    std::vector<VectorXf> positions;
    for(unsigned int i = 0; i < vertices.size(); i++) {
    	VectorXf pos = vertices[i].getPosition();
    	if(pos.size() == 3) {
            positions.push_back(pos);
    	} else {
            positions.push_back(Vector3f(pos.x(), pos.y(), 0));
    	}
    }
	if(vertices.size() > 0) {
		mDimensions = vertices[0].getNrOfDimensions();
		mBoundingBox = BoundingBox(positions);
	} else {
		mBoundingBox = BoundingBox(Vector3f(0,0,0)); // TODO Fix
	}
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
    mBoundingBox = BoundingBox(Vector3f(1,1,1));
    mIsInitialized = true;
    mNrOfConnections = nrOfTriangles;
    mDimensions = 3;
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
            std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
            mDataIsBeingWrittenTo = true;
    	}
        updateModifiedTimestamp();
    }
    if(!mVBOHasData) {
        // VBO has not allocated data: Create VBO
#ifdef FAST_MODULE_VISUALIZATION
#if defined(__APPLE__) || defined(__MACOSX)
#else
#if _WIN32
#else
        // If no Window is present, create a dummy gl context
        if(!QApplication::instance()) { // TODO make this work on all platforms
            Window::initializeQtApp();

            // Need a drawable for this to work
            //QGLWidget* widget = new QGLWidget;
            //widget->show();
            //widget->hide(); // TODO should probably delete widget as well
            //reportInfo() << "created a drawable" << Reporter::end;
        }
#endif
#endif
        QOpenGLFunctions_3_3_Core *fun = new QOpenGLFunctions_3_3_Core;
        fun->initializeOpenGLFunctions();
        fun->glGenBuffers(1, &mVBOID);
        fun->glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
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
            fun->glBufferData(GL_ARRAY_BUFFER, mNrOfConnections*18*sizeof(float), data, GL_STATIC_DRAW);
            delete[] data;
        } else {
            fun->glBufferData(GL_ARRAY_BUFFER, mNrOfConnections*18*sizeof(float), NULL, GL_STATIC_DRAW);
        }
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();
        if(glGetError() == GL_OUT_OF_MEMORY) {
        	throw Exception("OpenGL out of memory while creating mesh data for VBO");
        }
        //reportInfo() << "Created VBO with ID " << mVBOID << " and " << mNrOfConnections << " of triangles" << Reporter::end;
        // TODO Transfer data if any exist

        mVBOHasData = true;
        mVBODataIsUpToDate = true;
#else
        throw Exception("Creating mesh with VBO is disabled as FAST module visualization is disabled.");
#endif
    } else {
        if(!mVBODataIsUpToDate) {
            // TODO Update data
        }
    }

    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

	VertexBufferObjectAccess::pointer accessObject(new VertexBufferObjectAccess(mVBOID, mPtr.lock()));
	return std::move(accessObject);
}


// Hasher for the MeshVertex class
class KeyHasher {
    public:
        std::size_t operator()(const MeshVertex& v) const {

            // Start with a hash value of 0    .
            std::size_t seed = 0;

            // Modify 'seed' by XORing and bit-shifting in
            // one member of 'Key' after the other:
            // http://en.cppreference.com/w/cpp/utility/hash
            hash_combine(seed, std::hash<float>{}(v.getPosition().x()));
            hash_combine(seed, std::hash<float>{}(v.getPosition().y()));
            hash_combine(seed, std::hash<float>{}(v.getPosition().z()));

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
        throw Exception("Mesh has not been initialized.");
    }

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
    	{
    		std::lock_guard<std::mutex> lock(mDataIsBeingWrittenToMutex);
            mDataIsBeingWrittenTo = true;
    	}
        updateModifiedTimestamp();
    }
    if(!mHostHasData) {
#ifdef FAST_MODULE_VISUALIZATION
        // Host has not allocated data
    	if(mDimensions == 2)
    		throw Exception("Not implemented for 2D");
        // Get all vertices with normals from VBO (including duplicates)
        float* data = new float[mNrOfConnections*18];
        QOpenGLFunctions_3_3_Core *fun = new QOpenGLFunctions_3_3_Core;
        fun->initializeOpenGLFunctions();
        fun->glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        fun->glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*mNrOfConnections*18, data);
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        std::vector<MeshVertex> vertices;
        std::vector<VectorXui> triangles;
        std::unordered_map<MeshVertex, uint, KeyHasher> vertexList;
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
#else
        throw Exception("Creating mesh with VBO is disabled as FAST module visualization is disabled.");
#endif
    } else {
        if(!mHostDataIsUpToDate) {
            throw Exception("Not implemented yet!");
        }
    }

    {
        std::lock_guard<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

    MeshAccess::pointer accessObject(new MeshAccess(&mVertices,&mConnections,mPtr.lock()));
	return std::move(accessObject);
}

void Mesh::updateOpenCLBufferData(OpenCLDevice::pointer device) {
    // If buffer is up to date, no need to update
    if(mCLBuffersIsUpToDate.count(device) > 0 && mCLBuffersIsUpToDate[device] == true)
        return;

    if(!mHostHasData)
        throw Exception("Mesh currently need host data before getting CL buffer data");

    if(mCLBuffersIsUpToDate.count(device) == 0) {
        reportInfo() << "Creating OpenCL buffers for mesh" << reportEnd();
        // Allocate OpenCL buffers, need to know how many coordinates and how many connections
        size_t bufferSize = sizeof(float)*getDimensions()*mVertices.size();
        cl::Buffer* coordinatesBuffer = new cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, bufferSize);
        bufferSize = sizeof(int)*getDimensions()*mNrOfConnections;
        cl::Buffer* connectionsBuffer = new cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, bufferSize);
        mCoordinatesBuffers[device] = coordinatesBuffer;
        mConnectionBuffers[device] = connectionsBuffer;
    }

    if(mHostHasData && mHostDataIsUpToDate) {
        reportInfo() << "Transfer data from host to CL buffers" << reportEnd();
        // TODO Update data from host

        // Transfer coordinates
        cl::CommandQueue queue = device->getCommandQueue();
        {
            UniquePointer<float[]> coordinatesData(new float[getDimensions() * mVertices.size()]);
            int i = 0;
            for (MeshVertex vertex : mVertices) {
                coordinatesData[i] = vertex.getPosition().x();
                coordinatesData[i + 1] = vertex.getPosition().y();
                if (getDimensions() == 3) {
                    coordinatesData[i + 2] = vertex.getPosition().z();
                }
                i += getDimensions();
            }
            size_t bufferSize = sizeof(float) * getDimensions() * mVertices.size();
            queue.enqueueWriteBuffer(*mCoordinatesBuffers[device], CL_TRUE, 0, bufferSize, coordinatesData.get());
        }

        // Transfer connections
        {
            UniquePointer<uint[]> data(new uint[getDimensions() * mConnections.size()]);
            int i = 0;
            for (VectorXui connection : mConnections) {
                data[i] = connection.x();
                data[i + 1] = connection.y();
                if (getDimensions() == 3) {
                    data[i + 2] = connection.z();
                }
                i += getDimensions();
            }
            size_t bufferSize = sizeof(uint) * getDimensions() * mConnections.size();
            queue.enqueueWriteBuffer(*mConnectionBuffers[device], CL_TRUE, 0, bufferSize, data.get());
        }

    } else if(mVBOHasData && mVBODataIsUpToDate) {
        // TODO update data from VBO
        throw Exception("Mesh currently need host data before getting CL buffer data");
    } else {
        // No data to update from
    }

    mCLBuffersIsUpToDate[device] = true;
}

void Mesh::setAllDataToOutOfDate() {
    mHostDataIsUpToDate = false;
    mVBODataIsUpToDate = false;
    std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for(it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end(); it++) {
        it->second = false;
    }
}

MeshOpenCLAccess::pointer Mesh::getOpenCLAccess(accessType type, OpenCLDevice::pointer device) {
     if(!mIsInitialized) {
        throw Exception("Surface has not been initialized.");
    }

    blockIfBeingWrittenTo();

    if(type == ACCESS_READ_WRITE) {
    	blockIfBeingAccessed();
        std::unique_lock<std::mutex> lock(mDataIsBeingWrittenToMutex);
        mDataIsBeingWrittenTo = true;
    }
    updateOpenCLBufferData(device);
    if(type == ACCESS_READ_WRITE) {
        setAllDataToOutOfDate();
        updateModifiedTimestamp();
    }
    mCLBuffersIsUpToDate[device] = true;
    {
        std::unique_lock<std::mutex> lock(mDataIsBeingAccessedMutex);
        mDataIsBeingAccessed = true;
    }

    MeshOpenCLAccess::pointer accessObject(new MeshOpenCLAccess(mCoordinatesBuffers[device], mConnectionBuffers[device], mPtr.lock()));
	return std::move(accessObject);
}

Mesh::~Mesh() {
    freeAll();
}

Mesh::Mesh() {
    mIsInitialized = false;
    mVBOHasData = false;
    mVBODataIsUpToDate = false;
    mHostHasData = false;
    mHostDataIsUpToDate = false;
}

void Mesh::freeAll() {
    // TODO finish
    if(mVBOHasData) {
#ifdef FAST_MODULE_VISUALIZATION
        Window::getMainGLContext()->makeCurrent(); // Need an active context to delete the mesh VBO
        QOpenGLFunctions_3_3_Core *fun = new QOpenGLFunctions_3_3_Core;
        fun->initializeOpenGLFunctions();
        // glDeleteBuffer is not used due to multi-threading issues..
        fun->glDeleteBuffers(1, &mVBOID);

        // OLD delete method:
        //fun->glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        // This should delete the data, by replacing it with 1 byte buffer
        // Ideally it should be 0, but then the data is not deleted..
        //fun->glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
        //fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    }
    mVBOHasData = false;

    // For each CL buffer delete it
    std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for(it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end(); it++) {
        delete mConnectionBuffers[it->first];
        delete mCoordinatesBuffers[it->first];
    }
    mCoordinatesBuffers.clear();
    mConnectionBuffers.clear();
    mCLBuffersIsUpToDate.clear();

    mVertices.clear();
    mConnections.clear();
    mHostHasData = false;
}

void Mesh::free(ExecutionDevice::pointer device) {
    // TODO
    if(device->isHost()) {
        mVertices.clear();
        mConnections.clear();
        mHostHasData = false;
    } else {
        OpenCLDevice::pointer clDevice = device;
        if(mCLBuffersIsUpToDate.count(clDevice) > 0) {
            mCLBuffersIsUpToDate.erase(clDevice);
            delete mConnectionBuffers[clDevice];
            delete mCoordinatesBuffers[clDevice];
            mConnectionBuffers.erase(clDevice);
            mCoordinatesBuffers.erase(clDevice);
        }
    }
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


