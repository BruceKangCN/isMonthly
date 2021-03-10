#include "IsMonthly.hpp"

#include <QApplication>
#include <QTranslator>

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
    QString trPath;
    switch (language) {
    case 0: // Chinese_Simplified
        break;
    case 1:
        trPath = "translations/isMonthly_en";
        break;
    }
    if (translator.load(trPath)) {
        a.installTranslator(&translator);
    }

    isMonthly::IsMonthly w;
    w.show();

    return a.exec();
}
