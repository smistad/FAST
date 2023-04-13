#include "ButtonWidget.hpp"
#include <QHBoxLayout>
#include <QPushButton>
#include <sstream>
#include <iomanip>

namespace fast {

void ButtonWidget::init(std::string text, bool checked) {
    auto layout = new QHBoxLayout();
    setLayout(layout);
    m_button = new QPushButton();
    m_button->setText(QString::fromStdString(text));
    m_button->setCheckable(true);
    m_button->setChecked(checked);
    layout->addWidget(m_button);
    QObject::connect(m_button, &QPushButton::clicked, [=](bool checked) {
        clicked(checked);
    });
}

void ButtonWidget::clicked(bool checked) {
    m_button->setChecked(checked);
    if(m_callbackClass != nullptr) {
        m_callbackClass->handle(checked);
    } else {
        m_callbackFunction(checked);
    }
}

ButtonWidget::ButtonWidget(std::string text, bool checked,
                           std::function<void(bool)> callback, QWidget *parent) : QWidget(parent) {
    m_callbackFunction = callback;
    init(text, checked);
}

ButtonWidget::ButtonWidget(std::string text, bool checked,
                           ButtonWidgetCallback *callback, QWidget *parent) {
    m_callbackClass = callback;
    init(text, checked);
}

}