#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Подгружаем стили из ресурсов
    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        a.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    MainWindow w;
    w.show();
    return a.exec(); // В Qt5/Qt6 лучше использовать a.exec() вместо QApplication::exec()
}