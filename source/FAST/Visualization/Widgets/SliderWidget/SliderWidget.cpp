#include "SliderWidget.hpp"
#include <QSlider>
#include <QHBoxLayout>
#include <QLabel>
#include <sstream>
#include <iomanip>

namespace fast {

void SliderWidget::init(std::string name, float initial, float min, float max, float step) {
    auto layout = new QVBoxLayout();
    setLayout(layout);
    m_label = new QLabel();
    m_name = name;
    m_min = min;
    m_max = max;
    m_step = step;
    updateLabelWithValue(initial);
    layout->addWidget(m_label);
    m_slider = new QSlider(Qt::Horizontal);
    int steps = (max-min)/step;
    m_slider->setRange(0, steps);
    m_slider->setSliderPosition((initial - m_min)/m_step);
    layout->addWidget(m_slider);
    QObject::connect(m_slider, &QSlider::valueChanged, [=](int value) {
        valueChanged(value);
    });
}

void SliderWidget::updateLabelWithValue(float realValue) {
    std::stringstream stream;
    if(m_step == (int)m_step && m_min == (int)m_min) {
        stream << std::fixed << std::setprecision(0) << realValue;
    } else {
        stream << std::fixed << std::setprecision(2) << realValue; // TODO set nr of decimals in a smart way..
    }
    std::string s = stream.str();
    m_label->setText(QString::fromStdString(m_name + " (" + s + ")"));
}

void SliderWidget::valueChanged(int value) {
    float realValue = m_min + value*m_step;
    updateLabelWithValue(realValue);
    if(m_callbackClass != nullptr) {
        m_callbackClass->handle(realValue);
    } else {
        m_callbackFunction(realValue);
    }
}

SliderWidget::SliderWidget(std::string name, float initial, float min, float max, float step,
                           std::function<void(float)> callback, QWidget *parent) : QWidget(parent) {
    m_callbackFunction = callback;
    init(name, initial, min, max, step);
}

SliderWidget::SliderWidget(std::string name, float initial, float min, float max, float step,
                           SliderWidgetCallback *callback, QWidget *parent) {
    m_callbackClass = callback;
    init(name, initial, min, max, step);
}

}