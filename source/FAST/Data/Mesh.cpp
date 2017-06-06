#include "Mesh.hpp"
#include <thread>
#include "FAST/Utility.hpp"

#ifdef FAST_MODULE_VISUALIZATION
#include "FAST/Visualization/Window.hpp"
#include <QApplication>
#include <QGLFunctions>
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
        std::vector<MeshVertex> vertices,
        std::vector<MeshLine> lines,
        std::vector<MeshTriangle> triangles
    ) {
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
    std::vector<Vector3f> positions;
    for(unsigned int i = 0; i < vertices.size(); i++) {
    	VectorXf pos = vertices[i].getPosition();
        positions.push_back(pos);
    }
	if(vertices.size() > 0) {
		mBoundingBox = BoundingBox(positions);
	} else {
		mBoundingBox = BoundingBox(Vector3f(0,0,0)); // TODO Fix
	}
    mLines = lines;
    mTriangles = triangles;
    mNrOfTriangles = triangles.size();
    mHostHasData = true;
    mHostDataIsUpToDate = true;
    updateModifiedTimestamp();
}

void Mesh::create(unsigned int nrOfTriangles) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mBoundingBox = BoundingBox(Vector3f(1,1,1));
    mIsInitialized = true;
    mNrOfTriangles = nrOfTriangles;
}

VertexBufferObjectAccess::pointer Mesh::getVertexBufferObjectAccess(
        accessType type) {
    if(!mIsInitialized)
        throw Exception("Mesh has not been initialized.");

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
            //reportInfo() << "created a drawable" << Reporter::end();
        }
#endif
#endif
        QGLFunctions *fun = Window::getMainGLContext()->functions();
        fun->glGenBuffers(1, &mVBOID);
        fun->glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        if(mHostHasData) {
            // If host has data, transfer it.
            // Create data arrays with vertices and normals interleaved
            uint counter = 0;
            float* data = new float[mNrOfTriangles*18];
            for(uint i = 0; i < mNrOfTriangles; i++) {
                MeshTriangle triangle = mTriangles[i];
                for(uint j = 0; j < 3; j++) {
                    MeshVertex vertex = mVertices[triangle.getEndpoint(j)];
                    for(uint k = 0; k < 3; k++) {
                        data[counter+k] = vertex.getPosition()[k];
                        //reportInfo() << data[counter+k] << Reporter::end();
                        data[counter+3+k] = vertex.getNormal()[k];
                    }
                    //reportInfo() << "...." << Reporter::end();
                    counter += 6;
                }
            }
            fun->glBufferData(GL_ARRAY_BUFFER, mNrOfTriangles*18*sizeof(float), data, GL_STATIC_DRAW);
            delete[] data;
        } else {
            fun->glBufferData(GL_ARRAY_BUFFER, mNrOfTriangles*18*sizeof(float), NULL, GL_STATIC_DRAW);
        }
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();
        if(glGetError() == GL_OUT_OF_MEMORY) {
        	throw Exception("OpenGL out of memory while creating mesh data for VBO");
        }
        //reportInfo() << "Created VBO with ID " << mVBOID << " and " << mNrOfTriangles << " of triangles" << Reporter::end();

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
        // Get all vertices with normals from VBO (including duplicates)
        float* data = new float[mNrOfTriangles*18];
        QGLFunctions *fun = Window::getMainGLContext()->functions();
        fun->glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        fun->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*mNrOfTriangles*18, data);
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        std::vector<MeshVertex> vertices;
        std::vector<MeshTriangle> triangles;
        std::unordered_map<MeshVertex, uint, KeyHasher> vertexList;
        for(int t = 0; t < mNrOfTriangles; t++) {
            uint endpoints[3];
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
                //vertex.addConnection(t);

                // Only add if not a duplicate
                if(vertexList.count(vertex) > 0) {
                    // Found a duplicate
                    // Get index of duplicate
                    const uint duplicateIndex = vertexList[vertex];
                    // Add this triangle to the duplicate
                    MeshVertex& duplicate = vertices[duplicateIndex];
                    //duplicate.addConnection(t);
                    // Add the vertex to this triangle
                    endpoints[v] = duplicateIndex;
                } else {
                    // If duplicate was not found, add it to the list
                    vertices.push_back(vertex);
                    endpoints[v] = vertices.size()-1;
                    vertexList[vertex] = vertices.size()-1;
                }
            }
            MeshTriangle triangle(endpoints[0], endpoints[1], endpoints[2]);
            triangles.push_back(triangle);
        }
        mTriangles = triangles;
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

    MeshAccess::pointer accessObject(new MeshAccess(&mVertices, &mLines, &mTriangles, mPtr.lock()));
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
        size_t bufferSize = sizeof(float) * 3 * mVertices.size();
        cl::Buffer* coordinatesBuffer = new cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, bufferSize);
        mCoordinatesBuffers[device] = coordinatesBuffer;

        if(mLines.size() > 0) {
            bufferSize = sizeof(int) * 2 * mLines.size();
            cl::Buffer *lineBuffer = new cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, bufferSize);
            mLinesBuffers[device] = lineBuffer;
        } else {
            mLinesBuffers[device] = nullptr;
        }

        if(mTriangles.size() > 0) {
            bufferSize = sizeof(int) * 3 * mTriangles.size();
            cl::Buffer *triangleBuffer = new cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, bufferSize);
            mTrianglesBuffers[device] = triangleBuffer;
        } else {
            mTrianglesBuffers[device] = nullptr;
        }
    }

    if(mHostHasData && mHostDataIsUpToDate) {
        reportInfo() << "Transfer data from host to CL buffers" << reportEnd();
        // TODO Update data from host

        // Transfer coordinates
        cl::CommandQueue queue = device->getCommandQueue();
        {
            UniquePointer<float[]> coordinatesData(new float[3 * mVertices.size()]);
            int i = 0;
            for (MeshVertex vertex : mVertices) {
                coordinatesData[i] = vertex.getPosition().x();
                coordinatesData[i + 1] = vertex.getPosition().y();
                coordinatesData[i + 2] = vertex.getPosition().z();
                i += 3;
            }
            size_t bufferSize = sizeof(float) * 3 * mVertices.size();
            queue.enqueueWriteBuffer(*mCoordinatesBuffers[device], CL_TRUE, 0, bufferSize, coordinatesData.get());
        }

        // Transfer lines
        if(mLines.size() > 0) {
            UniquePointer<uint[]> data(new uint[2 * mLines.size()]);
            int i = 0;
            for(MeshLine line : mLines) {
                data[i] = line.getEndpoint1();
                data[i + 1] = line.getEndpoint2();
                i += 2;
            }
            size_t bufferSize = sizeof(uint) * 2 * mLines.size();
            queue.enqueueWriteBuffer(*mLinesBuffers[device], CL_TRUE, 0, bufferSize, data.get());
        }

        // Transfer triangles
        if(mTriangles.size() > 0) {
            UniquePointer<uint[]> data(new uint[3 * mTriangles.size()]);
            int i = 0;
            for(MeshTriangle triangle : mTriangles) {
                data[i] = triangle.getEndpoint1();
                data[i + 1] = triangle.getEndpoint2();
                data[i + 2] = triangle.getEndpoint3();
                i += 3;
            }
            size_t bufferSize = sizeof(uint) * 3 * mTriangles.size();
            queue.enqueueWriteBuffer(*mTrianglesBuffers[device], CL_TRUE, 0, bufferSize, data.get());
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

    MeshOpenCLAccess::pointer accessObject(new MeshOpenCLAccess(mCoordinatesBuffers[device], mLinesBuffers[device], mTrianglesBuffers[device], mPtr.lock()));
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
        // Need mutual exclusive write lock to delete data
        //VertexBufferObjectAccess::pointer access = getVertexBufferObjectAccess(ACCESS_READ_WRITE);
        Window::getMainGLContext()->makeCurrent(); // Need an active context to delete the mesh VBO
        QGLFunctions *fun = Window::getMainGLContext()->functions();
        // glDeleteBuffer is not used due to multi-threading issues..
        //fun->glDeleteBuffers(1, &mVBOID);

        // OLD delete method:
        fun->glBindBuffer(GL_ARRAY_BUFFER, mVBOID);
        // This should delete the data, by replacing it with 1 byte buffer
        // Ideally it should be 0, but then the data is not deleted..
        fun->glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        glFinish();
#endif
    }
    mVBOHasData = false;

    // For each CL buffer delete it
    std::unordered_map<OpenCLDevice::pointer, bool>::iterator it;
    for(it = mCLBuffersIsUpToDate.begin(); it != mCLBuffersIsUpToDate.end(); it++) {
        if(mLinesBuffers.count(it->first) > 0)
            delete mLinesBuffers[it->first];
        if(mTrianglesBuffers.count(it->first) > 0)
            delete mTrianglesBuffers[it->first];
        delete mCoordinatesBuffers[it->first];
    }
    mCoordinatesBuffers.clear();
    mLinesBuffers.clear();
    mTrianglesBuffers.clear();
    mCLBuffersIsUpToDate.clear();

    mVertices.clear();
    mLines.clear();
    mTriangles.clear();
    mHostHasData = false;
}

void Mesh::free(ExecutionDevice::pointer device) {
    // TODO
    if(device->isHost()) {
        mVertices.clear();
        mLines.clear();
        mTriangles.clear();
        mHostHasData = false;
    } else {
        OpenCLDevice::pointer clDevice = device;
        if(mCLBuffersIsUpToDate.count(clDevice) > 0) {
            mCLBuffersIsUpToDate.erase(clDevice);
            if(mLinesBuffers.count(clDevice) > 0)
                delete mLinesBuffers[clDevice];
            if(mTrianglesBuffers.count(clDevice) > 0)
                delete mTrianglesBuffers[clDevice];
            delete mCoordinatesBuffers[clDevice];
            mLinesBuffers.erase(clDevice);
            mTrianglesBuffers.erase(clDevice);
            mCoordinatesBuffers.erase(clDevice);
        }
    }
}

int Mesh::getNrOfTriangles() const {
    return mNrOfTriangles;
}

int Mesh::getNrOfVertices() const {
    return mVertices.size();
}

void Mesh::setBoundingBox(BoundingBox box) {
    mBoundingBox = box;
}

int Mesh::getNrOfLines() const {
	return mLines.size();
}

} // end namespace fast


