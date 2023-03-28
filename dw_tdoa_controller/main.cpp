// -------------------------------------------------------------------------------------------------------------------
//
//  File: main.cpp
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#include "RTLSControllerApplication.h"
#include "mainwindow.h"
#include <QSplashScreen>

#include <QFileInfo>
#include <QProcess>

extern Q_CORE_EXPORT QBasicAtomicInt qt_qhash_seed;

int main(int argc, char *argv[])
{

#if 0
//#if (REK_PRODUCT == 1)
    QFileInfo exec_fileinfo(argv[0]);
    QProcess  * leProcess = new QProcess();
    QString   Qle = exec_fileinfo.absolutePath()+"/LE.exe";

    system( "taskkill /f /im LE.exe");
    leProcess->start(Qle);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
//    qt_qhash_seed.store(12345);//this hack makes order of XML (QDomDocument) more deterministic

    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    RTLSControllerApplication app(argc, argv);

    QSplashScreen splash(QPixmap(":/splash.png"));

    splash.show();

    //app.sleep(5);

    splash.finish(app.mainWindow());

    app.mainWindow()->show();

    return app.QApplication::exec();
}
