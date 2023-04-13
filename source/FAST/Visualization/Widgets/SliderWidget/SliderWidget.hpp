#pragma once

#include <string>
#include <QWidget>
#include <FASTExport.hpp>

class QLabel;
class QSlider;

namespace fast {


#ifdef SWIG
%feature("director") SliderWidgetCallback;
%pythoncode %{
def SliderCallback(func):
    class CB(SliderWidgetCallback):
        def __init__(self):
            super().__init__()

        def handle(self, value):
            func(value)

    return CB()
%}
#endif

class SliderWidgetCallback {
    public:
        virtual void handle(float value) = 0;
        virtual ~SliderWidgetCallback() {};
};

class FAST_EXPORT SliderWidget : public QWidget {
    public:
        SliderWidget(std::string name, float initial, float min, float max, float step, std::function<void(float)> callback, QWidget* parent = nullptr);
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