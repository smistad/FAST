#ifndef VERTEXBUFFEROBJECTACCESS_HPP_
#define VERTEXBUFFEROBJECTACCESS_HPP_

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#else
#if _WIN32
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "FAST/Object.hpp"

namespace fast {

class Mesh;

class FAST_EXPORT VertexBufferObjectAccess {
    public:
        GLuint* getCoordinateVBO() const;
		GLuint* getNormalVBO() const;
		GLuint* getColorVBO() const;
		GLuint* getLineEBO() const;
		GLuint* getTriangleEBO() const;
		bool hasNormalVBO() const;
		bool hasColorVBO() const;
		bool hasEBO() const;
        VertexBufferObjectAccess(
				GLuint coordinateVBO,
				GLuint normalVBO,
				GLuint colorVBO,
				GLuint lineEBO,
				GLuint triangleEBO,
				bool useNormalVBO,
				bool useColorVBO,
				bool useEBO,
				SharedPointer<Mesh> mesh
		);
        void release();
        ~VertexBufferObjectAccess();
		typedef std::unique_ptr<VertexBufferObjectAccess> pointer;
    private:
        GLuint* mCoordinateVBO, *mNormalVBO, *mColorVBO, *mLineEBO, *mTriangleEBO;
		bool mUseNormalVBO, mUseColorVBO, mUseEBO;
        bool mIsDeleted;
        SharedPointer<Mesh> mMesh;
};

} // end namespace fast



#endif /* VERTEXBUFFEROBJECTACCESS_HPP_ */
