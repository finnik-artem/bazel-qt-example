#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo> // For QLibraryInfo::location

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("EnglishTutor"); // Good for QStandardPaths
    QApplication::setOrganizationName("MyCompany"); // Good for QStandardPaths

    QTranslator qtTranslator;
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (qtTranslator.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        a.installTranslator(&qtTranslator);
    }
    #else
    if (qtTranslator.load(QLocale::system(), "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
         a.installTranslator(&qtTranslator);
    }
    #endif


    MainWindow w;
    w.show();
    return a.exec();
}