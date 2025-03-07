#include <QApplication>
#include "mainwindow.h"
#include "build/ui_mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow win;
    win.show();
    return app.exec();
}