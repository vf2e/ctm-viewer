#include "MainWindow.h"

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QTimer>

#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkRenderingOpenGL);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("CTM Viewer"));
    app.setApplicationVersion(QStringLiteral("1.1.0"));
    app.setOrganizationName(QStringLiteral("Mag-aim"));

    const QIcon icon(QStringLiteral(":/icon.ico"));
    if (!icon.isNull()) {
        app.setWindowIcon(icon);
    }

    MainWindow window;
    window.show();

    QTimer::singleShot(0, &window, [&window]() {
        QString error;
        if (!window.loadDefaultSimulation(&error)) {
            QMessageBox::warning(&window, QStringLiteral("仿真加载失败"), error);
        }
    });

    if (argc > 1) {
        const QString filePath = QString::fromLocal8Bit(argv[1]);
        QTimer::singleShot(200, &window, [&window, filePath]() {
            window.openFile(filePath);
        });
    }

    return app.exec();
}
