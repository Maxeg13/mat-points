#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include "Window.h"


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
//    QPushButton button("Hello world!", nullptr);
//    button.resize(200, 100);
//    button.show();
    Window window = {};
    window.showFullScreen();
//    window.show();
    return QApplication::exec();
}
