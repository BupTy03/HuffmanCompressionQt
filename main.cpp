#include "mainwindow.hpp"
#include "huffmanencoding.hpp"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return QApplication::exec();
}
