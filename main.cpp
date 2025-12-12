#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

 // Windows 11 浅色主题调色板
    QPalette win11Palette;
    win11Palette.setColor(QPalette::Window, QColor(243, 243, 243));
    win11Palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    win11Palette.setColor(QPalette::Base, QColor(255, 255, 255));
    win11Palette.setColor(QPalette::AlternateBase, QColor(249, 249, 249));
    win11Palette.setColor(QPalette::Button, QColor(251, 251, 251));
    win11Palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    win11Palette.setColor(QPalette:: Highlight, QColor(0, 103, 192)); // Windows 11 蓝色
    win11Palette.setColor(QPalette::HighlightedText, Qt::white);

    a.setPalette(win11Palette);

    // Windows 11 风格 QSS
    a.setStyleSheet(R"(
        * {
            font-family: "Segoe UI Variable", "Segoe UI", sans-serif;
            font-size: 14px;
        }

        QPushButton {
            background-color: #FBFBFB;
            border: 1px solid #E5E5E5;
            border-radius: 4px;
            padding: 5px 16px;
            min-height: 32px;
        }

        QPushButton:hover {
            background-color: #F6F6F6;
        }

        QPushButton:pressed {
            background-color: #F0F0F0;
            color: rgba(0, 0, 0, 0.6);
        }

        /* 主要按钮 (Accent) */
        QPushButton[accent="true"] {
            background-color: #0067C0;
            color:  white;
            border: none;
        }

        QPushButton[accent="true"]:hover {
            background-color: #1975C5;
        }

        QLineEdit, QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #E5E5E5;
            border-bottom: 2px solid #0067C0;
            border-radius: 4px;
            padding: 6px 12px;
        }

        QCheckBox:: indicator {
            width: 20px;
            height: 20px;
            border-radius: 4px;
            border: 2px solid #848484;
        }

        QCheckBox::indicator:checked {
            background-color: #0067C0;
            border-color: #0067C0;
        }
    )");
    QTranslator translator;

    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MemRW_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
