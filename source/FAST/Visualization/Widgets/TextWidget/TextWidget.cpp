#include "TextWidget.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include <FAST/Utility.hpp>

namespace fast {

TextWidget::TextWidget(std::string text, QWidget *parent) : QWidget(parent) {
    m_text = text;
    m_label = new QLabel();
    auto layout = new QVBoxLayout;
    layout->addWidget(m_label);
    setLayout(layout);
    // Use QueuedConnection to assure slot is called in main thread
    connect(this, &TextWidget::repaintSignal, this, &TextWidget::repaintSlot, Qt::QueuedConnection);
}

void TextWidget::setText(std::string text) {
    m_text = text;
    emit repaintSignal();
}

std::string TextWidget::getText() const {
    return m_text;
}

void TextWidget::paintEvent(QPaintEvent *event) {
    // This should always run in main thread
    std::string text = m_text;
    for(const auto& var : m_variables) {
        text = replace(text, "{" + var.first + "}", var.second);
    }
    m_label->setText(text.c_str());
    QWidget::paintEvent(event);
}

void TextWidget::setVariable(std::string name, std::string value) {
    m_variables[name] = value;
    emit repaintSignal();
}

std::string TextWidget::getVariable(std::string name) const {
    return m_variables.at(name);
}

void TextWidget::repaintSlot() {
    update();
}

}
