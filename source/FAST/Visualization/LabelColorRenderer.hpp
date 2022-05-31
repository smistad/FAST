#pragma once

#include <FAST/Visualization/Renderer.hpp>
#include <FAST/Data/Color.hpp>

namespace fast {

class FAST_EXPORT LabelColorRenderer : public virtual Renderer {
    public:
        Color getColor(int label) const;
        void setColor(int label, Color color);
        void setColors(LabelColors colors);
        ~LabelColorRenderer();
    protected:
        void createColorUniformBufferObject();
        std::map<uint, Color> m_labelColors = {
                {1, Color::Green()},
                {2, Color::Blue()},
                {3, Color::Red()},
                {4, Color::Yellow()},
                {5, Color::Cyan()},
                {6, Color::Magenta()},
                {7, Color::Brown()},
                {255, Color::Cyan()},
        };
        bool m_colorsModified = true;
        uint m_colorsUBO;
        Color m_defaultColor = Color::Green();
    private:
};

}