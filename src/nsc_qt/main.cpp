#include "nsc_qt/main_window.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("clearCore-gui");
    app.setOrganizationName("nsc-qt");
    app.setApplicationVersion("1.1.0");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    nsc::qt::MainWindow window;
    window.show();

    return app.exec();
}
