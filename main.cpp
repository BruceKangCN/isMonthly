#include "widget.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // get language setting
    QSettings* config = new QSettings("app.ini", QSettings::IniFormat);
    int language = 0;
    if (config->contains("language")) {
        language = config->value("language").toInt();
    }
    delete config;

    // install translator
    QTranslator translator;
    switch (language) {
    case 0: // Chinese_Simplified
        break;
    case 1:
        translator.load("translations/isMonthly_en");
        break;
    }
    a.installTranslator(&translator);

    Widget w;
    w.show();

    return a.exec();
}
