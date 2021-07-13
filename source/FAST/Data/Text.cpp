#include "Text.hpp"

namespace fast {

Text::Text(std::string text, Color color) {
    m_text = text;
    m_color = color;
    updateModifiedTimestamp();
}

void Text::setText(std::string text) {
    m_text = text;
}

std::string Text::getText() const {
    return m_text;
}

void Text::setColor(Color color) {
    m_color = color;
}

Color Text::getColor() const {
    return m_color;
}

void Text::setPosition(Vector2f position) {
    Affine3f transform = Affine3f::Identity();
    transform.translate(Vector3f(position.x(), position.y(), 0.0f));
    getSceneGraphNode()->setTransform(transform);
}

void Text::setTextHeight(float millimeters) {
    if(millimeters <= 0.0f)
        throw Exception("Text height must be > 0 mm");
    m_textHeight = millimeters;
}

float Text::getTextHeight() const {
    return m_textHeight;
}

}