#include "LabelColorRenderer.hpp"

namespace fast {

void LabelColorRenderer::createColorUniformBufferObject() {
    if (m_colorsModified) {
        // Create UBO for colors
        glDeleteBuffers(1, &m_colorsUBO);
        glGenBuffers(1, &m_colorsUBO);
        int maxLabel = 0;
        for (auto&& labelColor : m_labelColors) {
            if (labelColor.first > maxLabel)
                maxLabel = labelColor.first;
        }
        auto colorData = std::make_unique<float[]>((maxLabel + 1) * 4);
        colorData[0] = 1.0f;
        colorData[1] = 0.0f;
        colorData[2] = 0.0f;
        colorData[3] = 1.0f;
        for (int i = 1; i <= maxLabel; ++i) {
            if (m_labelColors.count(i) > 0) {
                colorData[i * 4 + 0] = m_labelColors[i].getRedValue();
                colorData[i * 4 + 1] = m_labelColors[i].getGreenValue();
                colorData[i * 4 + 2] = m_labelColors[i].getBlueValue();
            }
            else {
                colorData[i * 4 + 0] = m_defaultColor.getRedValue();
                colorData[i * 4 + 1] = m_defaultColor.getGreenValue();
                colorData[i * 4 + 2] = m_defaultColor.getBlueValue();
            }
            colorData[i * 4 + 3] = 1.0f;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, m_colorsUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * (maxLabel + 1), colorData.get(), GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        m_colorsModified = false;
    }
}

void LabelColorRenderer::setColor(int label, Color color) {
    m_labelColors[label] = color;
    m_colorsModified = true;
    setModified(true);
}

LabelColorRenderer::~LabelColorRenderer() noexcept {
    glDeleteBuffers(1, &m_colorsUBO);
}

void LabelColorRenderer::setColors(std::map<uint, Color> colors) {
    for(auto&& item : colors) {
        m_labelColors[item.first] = item.second;
    }
    m_colorsModified = true;
    setModified(true);
}

Color LabelColorRenderer::getColor(int label) const {
    return m_labelColors.at(label);
}

}
