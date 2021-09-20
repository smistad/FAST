#pragma once

#include "SpatialDataObject.hpp"
#include "FAST/Data/Access/Access.hpp"
#include <vector>
#include "FAST/Data/DataTypes.hpp"
#include "FAST/Data/Access/VertexBufferObjectAccess.hpp"
#include "FAST/Data/Access/MeshAccess.hpp"
#include "FAST/Data/Access/MeshOpenCLAccess.hpp"
#include <condition_variable>
#include <unordered_map>

namespace fast {


/**
 * @brief Geometry data such as vertices, lines and triangles.
 *
 * The mesh data object contains vertices and optionally a set of lines and/or triangles.
 * Each vertex is represented as a MeshVertex and the lines and triangles as MeshLine and MeshTriangle respectively.
 *
 * @ingroup data
 */
class FAST_EXPORT Mesh : public SpatialDataObject {
    FAST_DATA_OBJECT(Mesh)
    public:
        /**
         * @brief Create a mesh
         *
         * @param vertices
         * @param lines
         * @param triangles
         */
        FAST_CONSTRUCTOR(Mesh, 
            std::vector<MeshVertex>, vertices, , 
            std::vector<MeshLine>, lines, = std::vector<MeshLine>(), 
            std::vector<MeshTriangle>, triangles, = std::vector<MeshTriangle>()
        )
#ifndef SWIG
        FAST_CONSTRUCTOR(Mesh, uint, nrOfVertices,, uint, nrOfLInes,, uint, nrOfTriangles,, bool, useColors,, bool, useNormals,, bool, useEBO,);
#endif
        VertexBufferObjectAccess::pointer getVertexBufferObjectAccess(accessType access);
        MeshAccess::pointer getMeshAccess(accessType access);
        MeshOpenCLAccess::pointer getOpenCLAccess(accessType access, OpenCLDevice::pointer device);
        int getNrOfTriangles();
        int getNrOfLines();
        int getNrOfVertices();
        void setBoundingBox(DataBoundingBox box);
        ~Mesh();
    private:
        Mesh();
        void freeAll();
        void free(ExecutionDevice::pointer device);
        void setAllDataToOutOfDate();
        void updateOpenCLBufferData(OpenCLDevice::pointer device);

        bool mIsInitialized;

        // VBO data
        bool mVBOHasData;
        bool mVBODataIsUpToDate;
        GLuint mCoordinateVBO = 0;
        GLuint mNormalVBO = 0;
        GLuint mColorVBO = 0;
        GLuint mLineEBO = 0;
        GLuint mTriangleEBO = 0;
        uint mNrOfVertices;
        uint mNrOfLines;
        uint mNrOfTriangles;
        bool mUseEBO;
        bool mUseColorVBO;
        bool mUseNormalVBO;

        // Host data
        bool mHostHasData;
        bool mHostDataIsUpToDate;
        std::vector<float> mCoordinates;
        std::vector<float> mNormals;
        std::vector<float> mColors;
        std::vector<uint> mLines;
        std::vector<uint> mTriangles;

        // OpenCL buffer data
        std::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mCoordinatesBuffers;
        std::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mLinesBuffers;
        std::unordered_map<OpenCLDevice::pointer, cl::Buffer*> mTrianglesBuffers;
        std::unordered_map<OpenCLDevice::pointer, bool> mCLBuffersIsUpToDate;

        // Declare as friends so they can get access to the accessFinished methods
        friend class MeshAccess;
        friend class VertexBufferObjectAccess;
        friend class MeshOpenCLAccess;
};

} // end namespace fast
