#include "FAST/Visualization/View.hpp"
#include <QWidget>
#include <QScopedPointer>
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>


class WindowWidget : public QWidget {
    public:
        WindowWidget() {
            setWindowTitle("Custom Qt Widget with FAST");
            resize(512, 512);

            // Create layout
            QVBoxLayout* layout = new QVBoxLayout;
            setLayout(layout);

            // Create button and add to layout
            QPushButton* button = new QPushButton("&Start", this);
            layout->addWidget(button);

            // Create a FAST view and add to layout
            fast::View* view = new fast::View;
            layout->addWidget(view);

            // TODO Create a simple FAST pipeline

            // TODO Connect the button to start pipeline

            // TODO Handle destruction some how
        }
    private:
};


int main(int argc, char** argv) {

    QScopedPointer<QApplication> app(new QApplication(argc, argv));

    QScopedPointer<WindowWidget> window(new WindowWidget);

    window->show();

    app->exec();

    return 0;
}
