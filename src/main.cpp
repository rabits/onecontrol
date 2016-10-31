#include "onecontrol.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDateTime>
#include <QGuiApplication>
#include <QFile>
#include <cstdio>

#ifdef Q_OS_ANDROID
#   include <android/log.h>
#endif

#include "settings.h"

#ifdef Q_OS_ANDROID
void printLogMessage(int priority, QString msg) { __android_log_print(priority, "onecontrol", msg.toLocal8Bit().constData()); }
#else
void printLogMessage(int, QString msg) { ::std::fprintf(stderr, "%s", msg.toLocal8Bit().constData()); }
#endif

void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    switch (type) {
        case QtDebugMsg:
            printLogMessage(3, QString("[OneControl %1] %2\n").arg(QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss.zzz")).arg(msg));
            break;
        case QtInfoMsg:
            printLogMessage(5, QString("[OneControl] Info: %1\n").arg(msg));
            break;
        case QtWarningMsg:
            printLogMessage(5, QString("[OneControl] Warning: %1\n").arg(msg));
            break;
        case QtCriticalMsg:
            printLogMessage(6, QString("[OneControl] Critical: %1\n").arg(msg));
            break;
        case QtFatalMsg:
            printLogMessage(7, QString("[OneControl] Fatal: %1\n").arg(msg));
            OneControl::destroyI();
            abort();
        default:
            printLogMessage(5, QString("[OneControl] Unknown: %1\n").arg(msg));
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    // Last argument is a configuration file
    if( app.arguments().count() > 1 && QFile::exists(app.arguments().last()) )
        Settings::setConfigFile(app.arguments().last());

    OneControl::I()->init(&app);

    qDebug("Application init done, starting");

    return app.exec();
}
