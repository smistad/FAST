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
			std::vector<uchar>* labels,
			std::vector<float>* scores,
			float* m_minimumSize,
			std::shared_ptr<BoundingBoxSet> bbset
		);
		void addBoundingBox(std::shared_ptr<BoundingBox> box);
		void addBoundingBox(Vector2f position, Vector2f size, uchar label, float score);
		std::vector<float> getCoordinates() const;
		std::vector<uint> getLines() const;
		std::vector<uchar> getLabels() const;
		std::vector<float> getScores() const;
		void addBoundingBoxes(std::vector<float> coordinates, std::vector<uint> lines, std::vector<uchar> labels, std::vector<float> scores, float minimumSize);
        void release();
        ~BoundingBoxSetAccess();
		typedef std::unique_ptr<BoundingBoxSetAccess> pointer;
		BoundingBoxSetAccess(const BoundingBoxSetAccess&) = delete;
	protected:
		std::vector<float>* m_coordinates;
		std::vector<uint>* m_lines;
		std::vector<uchar>* m_labels;
		std::vector<float>* m_scores;
		float* m_minimumSize;
		std::shared_ptr<BoundingBoxSet> m_bbset;
		bool m_released = false;
};

class FAST_EXPORT BoundingBoxSetOpenGLAccess {
	public:
		BoundingBoxSetOpenGLAccess(GLuint m_coordinatesVBO, GLuint m_linesEBO, GLuint m_labels, std::shared_ptr<BoundingBoxSet> bbset);
        GLuint getCoordinateVBO() const;
		GLuint getLinesEBO() const;
		GLuint getLabelVBO() const;
        void release();
        ~BoundingBoxSetOpenGLAccess();
		typedef std::unique_ptr<BoundingBoxSetOpenGLAccess> pointer;
		BoundingBoxSetOpenGLAccess(const BoundingBoxSetOpenGLAccess&) = delete;
	protected:
		GLuint m_coordinatesVBO, m_linesEBO, m_labelVBO;
        bool m_released = false;
		std::shared_ptr<BoundingBoxSet> m_bbset;
};

}