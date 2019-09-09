#include "mainwindow.hpp"
#include "huffmanencoding.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

//    compress_file("D:\\USER\\Documents\\text.txt", "D:\\USER\\Documents\\test_huffman.txt");
//    decompress_file("D:\\USER\\Documents\\test_huffman.txt", "D:\\USER\\Documents\\text_copy.txt");

    compress_file("D:\\USER\\Documents\\main.cpp", "D:\\USER\\Documents\\main_c.cpp");
    decompress_file("D:\\USER\\Documents\\main_c.cpp", "D:\\USER\\Documents\\main_d.cpp");

    return a.exec();
}
