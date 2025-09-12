#pragma once

#include <string>
#include <QWidget>
#include <FASTExport.hpp>

class QLabel;

namespace fast {


/**
 * @brief A text widget
 * @ingroup widgets
 */
class FAST_EXPORT TextWidget : public QWidget {
    Q_OBJECT
    public:
        /**
         * @brief Create a text widget
         * @param text The text to display. Can contain HTML and also variables, e.g. {this_var}.
         *      The values of the variables can be set with TextWidget::setVariable("this_var", "value");
         * @param parent
         */
        TextWidget(std::string text, QWidget* parent = nullptr);
        /**
         * @brief Set text
         * @param text The text to display. Can contain HTML and also variables, e.g. {this_var}.
         *      The values of the variables can be set with TextWidget::setVariable("this_var", "value");
         * @param text
         */
        void setText(std::string text);
        std::string getText() const;
        /**
         * @brief Set a variable
         * @param name  Name of variable, should exist as {<name>} in the text, and will be replaced with <value>
         * @param value  Value of variable
         */
        void setVariable(std::string name, std::string value);
        /**
         * @brief Set multiple variables
         * @param variables map of name and values of the variables
         */
        void setVariables(std::map<std::string, std::string> variables);
        std::string getVariable(std::string name) const;

#ifndef SWIG
        void repaintSlot();
    Q_SIGNALS:
        void repaintSignal();
#endif
    private:
        void paintEvent(QPaintEvent *event) override;
        std::string m_text;
        QLabel* m_label;
        std::map<std::string, std::string> m_variables;
};


}
