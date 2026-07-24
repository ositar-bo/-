#include "MainWindow.h"

#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("BIT P2P Communication"));
    app.setOrganizationName(QStringLiteral("Beijing Institute of Technology"));
    app.setFont(QFont(QStringLiteral("Noto Sans CJK SC"), 10));

    MainWindow window;
    window.show();
    return app.exec();
}
