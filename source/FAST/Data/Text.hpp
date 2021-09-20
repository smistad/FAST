#pragma once

#include <string>
#include <FAST/Data/Color.hpp>
#include <FAST/Data/SpatialDataObject.hpp>

namespace fast {

/**
 * @brief Text data object
 *
 * A data object which represents a string of text.
 *
 * @ingroup data
 */
class FAST_EXPORT Text : public SpatialDataObject {
	FAST_DATA_OBJECT(Text)
public:
    FAST_CONSTRUCTOR(Text,
                     std::string, text,,
                     Color, color, = Color::Green()
    );
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
