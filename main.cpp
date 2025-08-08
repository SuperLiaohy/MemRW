#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int get_addr_task();
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    get_addr_task();
    // QTranslator translator;
    // const QStringList uiLanguages = QLocale::system().uiLanguages();
    // for (const QString &locale : uiLanguages) {
    //     const QString baseName = "MemRW_" + QLocale(locale).name();
    //     if (translator.load(":/i18n/" + baseName)) {
    //         a.installTranslator(&translator);
    //         break;
    //     }
    // }
    // MainWindow w;
    // w.show();
    return a.exec();
}
