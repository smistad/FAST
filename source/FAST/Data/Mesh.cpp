#include "Mesh.hpp"
#include <thread>
#include "FAST/Utility.hpp"

#ifdef FAST_MODULE_VISUALIZATION
#include "FAST/Visualization/Window.hpp"
#include <QApplication>
#include <QGLFunctions>
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
        std::vector<MeshVertex> vertices,
        std::vector<MeshLine> lines,
        std::vector<MeshTriangle> triangles
    ) {
     if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    if(vertices.size() == 0) {
    	create(0, 0, 0, false, false, false);
    	return;
    }

    mIsInitialized = true;
    std::vector<Vector3f> positions;
    for(int i = 0; i < vertices.size(); i++) {
    	Vector3f pos = vertices[i].getPosition();
        positions.push_back(pos);
        mCoordinates.push_back(pos[0]);
        mCoordinates.push_back(pos[1]);
        mCoordinates.push_back(pos[2]);
        Vector3f normal = vertices[i].getNormal();
        mNormals.push_back(normal[0]);
        mNormals.push_back(normal[1]);
        mNormals.push_back(normal[2]);
        Color color = vertices[i].getColor();
        mColors.push_back(color.getRedValue());
        mColors.push_back(color.getGreenValue());
        mColors.push_back(color.getBlueValue());
    }
    for(auto line : lines) {
        mLines.push_back(line.getEndpoint1());
        mLines.push_back(line.getEndpoint2());
    }
    for(auto triangle : triangles) {
        mTriangles.push_back(triangle.getEndpoint1());
        mTriangles.push_back(triangle.getEndpoint2());
        mTriangles.push_back(triangle.getEndpoint3());
    }

	if(vertices.size() > 0) {
		mBoundingBox = BoundingBox(positions);
	} else {
		mBoundingBox = BoundingBox(Vector3f(0,0,0)); // TODO Fix
	}
    mNrOfVertices = vertices.size();
    mNrOfLines = lines.size();
    mNrOfTriangles = triangles.size();
    mUseColorVBO = true;
    mUseNormalVBO = true;
    mUseEBO = true;
    mHostHasData = true;
    mHostDataIsUpToDate = true;
    updateModifiedTimestamp();
}

void Mesh::create(
        uint nrOfVertices,
        uint nrOfLines,
        uint nrOfTriangles,
        bool useColors,
        bool useNormals,
        bool useEBO
        ) {
    if(mIsInitialized) {
        // Delete old data
        freeAll();
    }
    mBoundingBox = BoundingBox(Vector3f(1,1,1));
    mIsInitialized = true;
    mNrOfVertices = nrOfVertices;
    mNrOfLines = nrOfLines;
    mUseColorVBO = useColors;
    mUseNormalVBO = useNormals;
    mUseEBO = useEBO;
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
        // Make sure GL context is available
        if(!QApplication::instance()) {
            Window::initializeQtApp();
        }
        if(QGLContext::currentContext() == nullptr)
            Window::getMainGLContext()->makeCurrent();
        QGLFunctions *fun = Window::getMainGLContext()->functions();
        fun->glGenBuffers(1, &mCoordinateVBO);
        if(mHostHasData) {
            fun->glGenBuffers(1, &mNormalVBO);
            fun->glGenBuffers(1, &mColorVBO);
            fun->glGenBuffers(1, &mLineEBO);
            fun->glGenBuffers(1, &mTriangleEBO);
            // If host has data, transfer it.
            // Coordinates
            fun->glBindBuffer(GL_ARRAY_BUFFER, mCoordinateVBO);
            fun->glBufferData(GL_ARRAY_BUFFER, mCoordinates.size()*sizeof(float), mCoordinates.data(), GL_STATIC_DRAW);
            // Normals
            fun->glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
            fun->glBufferData(GL_ARRAY_BUFFER, mNormals.size()*sizeof(float), mNormals.data(), GL_STATIC_DRAW);
            // Color
            fun->glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
            fun->glBufferData(GL_ARRAY_BUFFER, mColors.size()*sizeof(float), mColors.data(), GL_STATIC_DRAW);
            // Line EBO
            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLineEBO);
            fun->glBufferData(GL_ELEMENT_ARRAY_BUFFER, mLines.size()*sizeof(uint), mLines.data(), GL_STATIC_DRAW);
            // Triangle EBO
            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleEBO);
            fun->glBufferData(GL_ELEMENT_ARRAY_BUFFER, mTriangles.size()*sizeof(uint), mTriangles.data(), GL_STATIC_DRAW);
        } else {
            // Only allocate space
            fun->glBindBuffer(GL_ARRAY_BUFFER, mCoordinateVBO);
            fun->glBufferData(GL_ARRAY_BUFFER, mNrOfVertices*3*sizeof(float), NULL, GL_STATIC_DRAW);
            if(mUseNormalVBO) {
                fun->glGenBuffers(1, &mNormalVBO);
                fun->glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
                fun->glBufferData(GL_ARRAY_BUFFER, mNrOfVertices * 3 * sizeof(float), NULL, GL_STATIC_DRAW);
            }
            if(mUseColorVBO) {
                fun->glGenBuffers(1, &mColorVBO);
                fun->glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
                fun->glBufferData(GL_ARRAY_BUFFER, mNrOfVertices * 3 * sizeof(float), NULL, GL_STATIC_DRAW);
            }
            if(mUseEBO) {
                  // Line EBO
                fun->glGenBuffers(1, &mLineEBO);
                fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLineEBO);
                fun->glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNrOfLines*sizeof(uint), NULL, GL_STATIC_DRAW);
                // Triangle EBO
                fun->glGenBuffers(1, &mTriangleEBO);
                fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleEBO);
                fun->glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNrOfTriangles*sizeof(uint), NULL, GL_STATIC_DRAW);
            }
        }
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

	VertexBufferObjectAccess::pointer accessObject(
            new VertexBufferObjectAccess(
                    mCoordinateVBO,
                    mNormalVBO,
                    mColorVBO,
                    mLineEBO,
                    mTriangleEBO,
                    mUseNormalVBO,
                    mUseColorVBO,
                    mUseEBO,
                    std::static_pointer_cast<Mesh>(mPtr.lock())
            )
    );
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
        // Get all mesh data from VBOs
        if(!mVBOHasData)
            throw Exception("No data available");

        // Make sure GL context is available
        if(!QApplication::instance()) {
            Window::initializeQtApp();
        }
        if(QGLContext::currentContext() == nullptr)
            Window::getMainGLContext()->makeCurrent();

        QOpenGLFunctions_3_3_Core *fun = new QOpenGLFunctions_3_3_Core;
        fun->initializeOpenGLFunctions();

        mCoordinates.resize(mNrOfVertices*3);
        fun->glBindBuffer(GL_ARRAY_BUFFER, mCoordinateVBO);
        fun->glGetBufferSubData(GL_ARRAY_BUFFER, 0, mNrOfVertices*3*sizeof(float), mCoordinates.data());
        mNormals.resize(mNrOfVertices*3);
        if(mUseNormalVBO) {
            fun->glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
            fun->glGetBufferSubData(GL_ARRAY_BUFFER, 0, mNrOfVertices * 3 * sizeof(float), mNormals.data());
        }
        mColors.resize(mNrOfVertices*3);
        if(mUseColorVBO) {
            fun->glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
            fun->glGetBufferSubData(GL_ARRAY_BUFFER, 0, mNrOfVertices * 3 * sizeof(float), mColors.data());
        }
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);

        mLines.resize(mNrOfLines*2);
        mTriangles.resize(mNrOfTriangles*3);
        if(mUseEBO) {
              // Line EBO
            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLineEBO);
            fun->glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mNrOfLines*sizeof(uint), mLines.data());

            // Triangle EBO
            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleEBO);
            fun->glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mNrOfTriangles*sizeof(uint), mTriangles.data());

            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        } else {
            // If EBO is not used, it is assumed that all vertices define consecutive lines/triangles
            uint counter = 0;
            for(int i = 0; i < mNrOfLines; ++i) {
                mLines[i*2] = counter;
                mLines[i*2] = counter+1;
                counter += 2;
            }
            counter = 0;
            for(int i = 0; i < mNrOfTriangles; ++i) {
                mTriangles[i*3] = counter;
                mTriangles[i*3+1] = counter+1;
                mTriangles[i*3+2] = counter+2;
                counter += 3;
            }
        }


        mHostHasData = true;
        mHostDataIsUpToDate = true;
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

    MeshAccess::pointer accessObject(new MeshAccess(&mCoordinates, &mNormals, &mColors, &mLines, &mTriangles, std::static_pointer_cast<Mesh>(mPtr.lock())));
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
        size_t bufferSize = sizeof(float) * 3 * mNrOfVertices;
        cl::Buffer* coordinatesBuffer = new cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, bufferSize);
        mCoordinatesBuffers[device] = coordinatesBuffer;

        if(mLines.size() > 0) {
            bufferSize = sizeof(int) * 2 * mNrOfLines;
            cl::Buffer *lineBuffer = new cl::Buffer(device->getContext(), CL_MEM_READ_WRITE, bufferSize);
            mLinesBuffers[device] = lineBuffer;
        } else {
            mLinesBuffers[device] = nullptr;
        }

        if(mTriangles.size() > 0) {
            bufferSize = sizeof(int) * 3 * mNrOfTriangles;
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
        queue.enqueueWriteBuffer(*mCoordinatesBuffers[device], CL_TRUE, 0, mNrOfVertices*3*sizeof(float), mCoordinates.data());

        // Transfer lines
        if(mLines.size() > 0) {
            queue.enqueueWriteBuffer(*mLinesBuffers[device], CL_TRUE, 0, mNrOfLines*2*sizeof(uint), mLines.data());
        }

        // Transfer triangles
        if(mTriangles.size() > 0) {
            queue.enqueueWriteBuffer(*mTrianglesBuffers[device], CL_TRUE, 0, mNrOfTriangles*3*sizeof(uint), mTriangles.data());
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

    MeshOpenCLAccess::pointer accessObject(new MeshOpenCLAccess(mCoordinatesBuffers[device], mLinesBuffers[device], mTrianglesBuffers[device], std::static_pointer_cast<Mesh>(mPtr.lock())));
	return std::move(accessObject);
}

Mesh::~Mesh() {
    freeAll();
}

Mesh::Mesh() {
    mIsInitialized = false;
    mVBOHasData = false;
    mVBODataIsUpToDate = false;
    mUseColorVBO = false;
    mUseNormalVBO = false;
    mUseEBO = false;
    mHostHasData = false;
    mHostDataIsUpToDate = false;
}

void Mesh::freeAll() {
    // TODO finish
    if(mVBOHasData) {
#ifdef FAST_MODULE_VISUALIZATION
        // Need mutual exclusive write lock to delete data
        //VertexBufferObjectAccess::pointer access = getVertexBufferObjectAccess(ACCESS_READ_WRITE);
        //Window::getMainGLContext()->makeCurrent(); // Need an active context to delete the mesh VBO
        QGLFunctions *fun = Window::getMainGLContext()->functions();
        // glDeleteBuffer is not used due to multi-threading issues..
        //fun->glDeleteBuffers(1, &mVBOID);

        // OLD delete method:
        // This should delete the data, by replacing it with 1 byte buffer
        // Ideally it should be 0, but then the data is not deleted..
        fun->glBindBuffer(GL_ARRAY_BUFFER, mCoordinateVBO);
        fun->glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
        if(mUseNormalVBO) {
            fun->glBindBuffer(GL_ARRAY_BUFFER, mNormalVBO);
            fun->glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
        }
        if(mUseColorVBO) {
            fun->glBindBuffer(GL_ARRAY_BUFFER, mColorVBO);
            fun->glBufferData(GL_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
        }
        fun->glBindBuffer(GL_ARRAY_BUFFER, 0);
        if(mUseEBO) {
            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLineEBO);
            fun->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTriangleEBO);
            fun->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 1, NULL, GL_STATIC_DRAW);
            fun->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
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

    mCoordinates.clear();
    mNormals.clear();
    mColors.clear();
    mLines.clear();
    mTriangles.clear();
    mHostHasData = false;
}

void Mesh::free(ExecutionDevice::pointer device) {
    // TODO
    if(device->isHost()) {
        mCoordinates.clear();
        mNormals.clear();
        mColors.clear();
        mLines.clear();
        mTriangles.clear();
        mHostHasData = false;
    } else {
        OpenCLDevice::pointer clDevice = std::static_pointer_cast<OpenCLDevice>(device);
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

int Mesh::getNrOfTriangles() {
    if(mHostHasData && mHostDataIsUpToDate)
        mNrOfTriangles = mTriangles.size() / 3;
    return mNrOfTriangles;
}

int Mesh::getNrOfVertices() {
    if(mHostHasData && mHostDataIsUpToDate)
        mNrOfVertices = mCoordinates.size() / 3;

    return mNrOfVertices;
}

void Mesh::setBoundingBox(BoundingBox box) {
    mBoundingBox = box;
}

int Mesh::getNrOfLines() {
    if(mHostHasData && mHostDataIsUpToDate)
        mNrOfLines = mLines.size() / 2;
	return mNrOfLines;
}

} // end namespace fast


