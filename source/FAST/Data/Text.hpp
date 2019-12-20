#pragma once

#include <string>
#include <FAST/Data/Color.hpp>
#include <FAST/Data/SpatialDataObject.hpp>

namespace fast {

// A macro for creating new simple data objects
class FAST_EXPORT Text : public SpatialDataObject {
	FAST_OBJECT(Text)
public:
    void create(std::string text, Color color = Color::Green());
	void setPosition(Vector2f position);
    void free(ExecutionDevice::pointer device) {};
    void freeAll() {};
    std::string getText() const;
    void setText(std::string text);
    Color getColor() const;
    void setColor(Color color);
    void setTextHeight(float millimeters);
    float getTextHeight() const;
protected:
	Text() {};
	std::string m_text = "";
	Color m_color = Color::Green();
	float m_textHeight = 3.0f;
};

}
