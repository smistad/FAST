#include "BoundingBoxSetAccess.hpp"
#include <FAST/Data/BoundingBox.hpp>
#include <FAST/Reporter.hpp>

namespace fast {

BoundingBoxSetAccess::BoundingBoxSetAccess(
	std::vector<float>* coordinates,
	std::vector<uint>* lines,
	std::vector<uchar>* labels,
	SharedPointer<BoundingBoxSet> bbset
	) : m_coordinates(coordinates), m_lines(lines), m_labels(labels), m_bbset(bbset) {

}


void BoundingBoxSetAccess::addBoundingBox(BoundingBox::pointer box) {
	addBoundingBox(box->getPosition(), box->getSize(), box->getLabel());
}

void BoundingBoxSetAccess::addBoundingBox(Vector2f position, Vector2f size, uchar label = 0) {
	if(!m_released) {
		int count = m_coordinates->size() / 3;

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

		m_labels->push_back(label);
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

void BoundingBoxSetAccess::addBoundingBoxes(std::vector<float> coordinates, std::vector<uint> lines) {
	const int size = m_coordinates->size() / 3;
	m_coordinates->insert(m_coordinates->end(), coordinates.begin(), coordinates.end());
	// Have to update indexes of new lines:
	std::transform(lines.begin(), lines.end(), lines.begin(), [size](uint index) -> uint {
		return index + size;
	});
	m_lines->insert(m_lines->end(), lines.begin(), lines.end());
}


void BoundingBoxSetAccess::release() {
	m_bbset->accessFinished();
	m_released = true;
}

BoundingBoxSetAccess::~BoundingBoxSetAccess() {
	release();
}

BoundingBoxSetOpenGLAccess::BoundingBoxSetOpenGLAccess(GLuint coordinatesVBO, GLuint linesEBO, GLuint labelVBO, SharedPointer<BoundingBoxSet> bbset) : 
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