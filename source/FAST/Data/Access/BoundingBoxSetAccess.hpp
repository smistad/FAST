#pragma once

#include <FAST/Data/Access/Access.hpp>
#include <FAST/Object.hpp>
#include <vector>
#include <FAST/Data/DataTypes.hpp>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif


namespace fast {

class BoundingBox;
class BoundingBoxSet;

class FAST_EXPORT BoundingBoxSetAccess {
	public:
		BoundingBoxSetAccess(
			std::vector<float>* coordinates,
			std::vector<uint>* lines,
			SharedPointer<BoundingBoxSet> bbset
		);
		void addBoundingBox(SharedPointer<BoundingBox> box);
		void addBoundingBox(Vector2f position, Vector2f size);
		std::vector<float> getCoordinates() const;
		std::vector<uint> getLines() const;
		void addBoundingBoxes(std::vector<float> coordinates, std::vector<uint> lines);
        void release();
        ~BoundingBoxSetAccess();
		typedef std::unique_ptr<BoundingBoxSetAccess> pointer;
		BoundingBoxSetAccess(const BoundingBoxSetAccess&) = delete;
	protected:
		std::vector<float>* m_coordinates;
		std::vector<uint>* m_lines;
		SharedPointer<BoundingBoxSet> m_bbset;
		bool m_released = false;
};

class FAST_EXPORT BoundingBoxSetOpenGLAccess {
	public:
		BoundingBoxSetOpenGLAccess(GLuint m_coordinatesVBO, GLuint m_linesEBO, SharedPointer<BoundingBoxSet> bbset);
        GLuint getCoordinateVBO() const;
		GLuint getLinesEBO() const;
        void release();
        ~BoundingBoxSetOpenGLAccess();
		typedef std::unique_ptr<BoundingBoxSetOpenGLAccess> pointer;
		BoundingBoxSetOpenGLAccess(const BoundingBoxSetOpenGLAccess&) = delete;
	protected:
		GLuint m_coordinatesVBO, m_linesEBO;
        bool m_released = false;
		SharedPointer<BoundingBoxSet> m_bbset;
};

}