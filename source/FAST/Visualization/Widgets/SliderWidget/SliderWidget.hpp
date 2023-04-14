#pragma once

#include <string>
#include <QWidget>
#include <FASTExport.hpp>
#include <iostream>

class QLabel;
class QSlider;

namespace fast {


#ifdef SWIG
%feature("director") SliderWidgetCallback;
%pythoncode %{
_slider_callbacks = [] # Hack to avoid callbacks being deleted
def SliderCallback(func):
    global _slider_callbacks
    class CB(SliderWidgetCallback):
        def __init__(self):
            super().__init__()

        def handle(self, value):
            func(value)
    obj = CB()
    _slider_callbacks.append(obj)
    return obj
%}
#endif

class SliderWidgetCallback {
    public:
        virtual void handle(float value) = 0;
        virtual ~SliderWidgetCallback() { std::cout << "Destroying SliderWidgetCallback" << std::endl;};
};

class FAST_EXPORT SliderWidget : public QWidget {
    public:
#ifndef SWIG
        SliderWidget(std::string name, float initial, float min, float max, float step, std::function<void(float)> callback, QWidget* parent = nullptr);
#endif
        SliderWidget(std::string name, float initial, float min, float max, float step, SliderWidgetCallback* callback, QWidget* parent = nullptr);
        void valueChanged(int value);
    private:
        void updateLabelWithValue(float realValue);
        void init(std::string name, float current, float min, float max, float step);
        std::function<void(float)> m_callbackFunction;
        SliderWidgetCallback* m_callbackClass = nullptr;

        QLabel* m_label;
        QSlider* m_slider;
        std::string m_name;
        float m_min;
        float m_max;
        float m_step;

};


}