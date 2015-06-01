#include "qtInteroperability.hpp"

int main(int argc, char** argv) {

    QScopedPointer<QApplication> app(new QApplication(argc, argv));

    QScopedPointer<WindowWidget> window(new WindowWidget);

    window->show();

    app->exec();

    return 0;
}
