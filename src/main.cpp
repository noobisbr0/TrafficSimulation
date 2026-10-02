#include "ui/mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char* argv[]) {
    // 1. Отключаем D-Bus опрос демона GVFS в GLib/GIO (главная причина 10с таймаута файловых окон)
    qputenv("GIO_USE_VFS", "local");

    // 2. Отключаем шину доступности AT-SPI, которая блокирует вызов любых QDialog/QMessageBox
    qputenv("NO_AT_BRIDGE", "1");
    qputenv("QT_ACCESSIBILITY", "0");
    qputenv("QT_LINUX_ACCESSIBILITY_ALWAYS_ON", "0");

    // 3. Отключаем синхронные запросы Wayland к XDG Desktop Portal
    qputenv("QT_USE_PORTAL", "0");
    qputenv("QT_NO_XDG_DESKTOP_PORTAL", "1");
    qputenv("QT_QPA_PLATFORMTHEME", "generic");

    QApplication app(argc, argv);

    MainWindow window;

    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        window.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    window.show();
    return app.exec();
}