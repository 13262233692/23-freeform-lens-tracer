#include <QApplication>
#include <QSurfaceFormat>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setSamples(4);
    format.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication::setStyle("Fusion");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(45, 45, 48));
    palette.setColor(QPalette::WindowText, QColor(220, 220, 220));
    palette.setColor(QPalette::Base, QColor(30, 30, 30));
    palette.setColor(QPalette::AlternateBase, QColor(45, 45, 48));
    palette.setColor(QPalette::ToolTipBase, QColor(25, 25, 25));
    palette.setColor(QPalette::ToolTipText, QColor(220, 220, 220));
    palette.setColor(QPalette::Text, QColor(220, 220, 220));
    palette.setColor(QPalette::Button, QColor(55, 55, 58));
    palette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
    palette.setColor(QPalette::BrightText, QColor(255, 50, 50));
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    app.setPalette(palette);

    MainWindow window;
    window.show();

    return app.exec();
}
