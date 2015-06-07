/**
 * Examples/Interoperability/qtInteroperability.cpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "qtInteroperability.hpp"

int main(int argc, char** argv) {

    // Create Qt Application
    QScopedPointer<QApplication> app(new QApplication(argc, argv));

    // Create Qt widget
    QScopedPointer<WindowWidget> window(new WindowWidget);

    // Show widget
    window->show();

    // Start application event handling (this call blocks until application is closed)
    app->exec();

    return 0;
}
