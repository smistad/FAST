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
    public:
        TextWidget(std::string text, QWidget* parent = nullptr);
        void setText(std::string text);
        std::string getText() const;
        void setVariable(std::string name, std::string value);
        std::string getVariable(std::string name) const;
    private:
        void paintEvent(QPaintEvent *event) override;
        std::string m_text;
        QLabel* m_label;
        std::map<std::string, std::string> m_variables;
};


}
