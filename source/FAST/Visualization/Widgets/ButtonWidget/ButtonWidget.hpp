#pragma once

#include <string>
#include <QWidget>
#include <FASTExport.hpp>

class QLabel;
class QPushButton;

namespace fast {


#ifdef SWIG
%feature("director") ButtonWidgetCallback;

%pythoncode %{
_button_callbacks = [] # Hack to avoid callbacks being deleted
def ButtonCallback(func):
    class CB(ButtonWidgetCallback):
        def __init__(self):
            super().__init__()

        def handle(self, value):
            func(value)

    obj = CB()
    _slider_callbacks.append(obj)
    return obj
%}
#endif

class ButtonWidgetCallback {
    public:
        virtual void handle(bool checked) = 0;
        virtual ~ButtonWidgetCallback() {};
};

class FAST_EXPORT ButtonWidget : public QWidget {
    public:
#ifndef SWIG
        ButtonWidget(std::string text, bool checked, std::function<void(bool)> callback, QWidget* parent = nullptr);
#endif
        ButtonWidget(std::string text, bool checked, ButtonWidgetCallback* callback, QWidget* parent = nullptr);
        void clicked(bool check);
    private:
        void init(std::string text, bool checked);
        std::function<void(float)> m_callbackFunction;
        ButtonWidgetCallback* m_callbackClass = nullptr;

        QPushButton* m_button;
};


}