#include "BoundingBoxSetAccess.hpp"
#include <FAST/Data/BoundingBox.hpp>
#include <FAST/Reporter.hpp>

namespace fast {

BoundingBoxSetAccess::BoundingBoxSetAccess(
	std::vector<float>* coordinates,
	std::vector<uint>* lines,
	std::vector<uchar>* labels,
	std::vector<float>* scores,
	float* minimumSize,
	std::shared_ptr<BoundingBoxSet> bbset
	) : m_coordinates(coordinates), m_lines(lines), m_labels(labels), m_scores(scores), m_bbset(bbset), m_minimumSize(minimumSize) {

}


void BoundingBoxSetAccess::addBoundingBox(BoundingBox::pointer box) {
	addBoundingBox(box->getPosition(), box->getSize(), box->getLabel(), box->getScore());
}

void BoundingBoxSetAccess::addBoundingBox(Vector2f position, Vector2f size, uchar label, float score) {
	if(!m_released) {
		int count = m_coordinates->size() / 3;

		*m_minimumSize = std::min(*m_minimumSize, size.x());
        *m_minimumSize = std::min(*m_minimumSize, size.y());

		// Add the four corners of a bounding box
		m_coordinates->push_back(position.x());
		m_coordinates->push_back(position.y());
		m_coordinates->push_back(0);

		m_coordinates->push_back(position.x() + size.x());
		m_coordinates->push_back(position.y());
		m_coordinates->push_back(0);

		m_coordinates->push_back(position.x() + size.x());
		m_coordinates->push_back(position.y() + size.y());
		m_coordinates->push_back(0);

		m_coordinates->push_back(position.x());
		m_coordinates->push_back(position.y() + size.y());
		m_coordinates->push_back(0);

		// Lines are pairs (from,to)
		m_lines->push_back(count);
		m_lines->push_back(count + 1);
		m_lines->push_back(count + 1);
		m_lines->push_back(count + 2);
		m_lines->push_back(count + 2);
		m_lines->push_back(count + 3);
		m_lines->push_back(count + 3);
		m_lines->push_back(count);

		// Add label four times, once for each vertex
		m_labels->push_back(label);
		m_labels->push_back(label);
		m_labels->push_back(label);
		m_labels->push_back(label);

		m_scores->push_back(score);
	} else {
		Reporter::warning() << "Bounding box set access was released, but was accessed." << Reporter::end();
	}
}


std::vector<float> BoundingBoxSetAccess::getCoordinates() const {
	return *m_coordinates;
}

std::vector<uint> BoundingBoxSetAccess::getLines() const {
	return *m_lines;
}

std::vector<uchar> BoundingBoxSetAccess::getLabels() const {
	return *m_labels;
}

std::vector<float> BoundingBoxSetAccess::getScores() const {
	return *m_scores;
}

void BoundingBoxSetAccess::addBoundingBoxes(std::vector<float> coordinates, std::vector<uint> lines, std::vector<uchar> labels, std::vector<float> scores, float minimumSize) {
	const int size = m_coordinates->size() / 3;
	m_coordinates->insert(m_coordinates->end(), coordinates.begin(), coordinates.end());
	// Have to update indexes of new lines:
	std::transform(lines.begin(), lines.end(), lines.begin(), [size](uint index) -> uint {
		return index + size;
	});
	*m_minimumSize = std::min(*m_minimumSize, minimumSize);
	m_lines->insert(m_lines->end(), lines.begin(), lines.end());
	m_labels->insert(m_labels->end(), labels.begin(), labels.end());
	m_scores->insert(m_scores->end(), scores.begin(), scores.end());
}


void BoundingBoxSetAccess::release() {
	m_bbset->accessFinished();
	m_released = true;
}

BoundingBoxSetAccess::~BoundingBoxSetAccess() {
	release();
}

BoundingBoxSetOpenGLAccess::BoundingBoxSetOpenGLAccess(GLuint coordinatesVBO, GLuint linesEBO, GLuint labelVBO, std::shared_ptr<BoundingBoxSet> bbset) : 
	m_coordinatesVBO(coordinatesVBO), m_linesEBO(linesEBO), m_labelVBO(labelVBO), m_bbset(bbset) {

}

GLuint BoundingBoxSetOpenGLAccess::getCoordinateVBO() const {
	if(m_released)
		throw Exception("BoundingBoxSet OpenGL access was released.");
	return m_coordinatesVBO;
}

GLuint BoundingBoxSetOpenGLAccess::getLinesEBO() const {
	if(m_released)
		throw Exception("BoundingBoxSet OpenGL access was released.");
	return m_linesEBO;
}

GLuint BoundingBoxSetOpenGLAccess::getLabelVBO() const {
	if(m_released)
		throw Exception("BoundingBoxSet OpenGL access was released.");
	return m_labelVBO;
}

void BoundingBoxSetOpenGLAccess::release() {
	m_bbset->accessFinished();
	m_released = true;
}

BoundingBoxSetOpenGLAccess::~BoundingBoxSetOpenGLAccess() {
	release();
}

}