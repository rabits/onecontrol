#ifndef ONECONTROL_H
#define ONECONTROL_H

#include <QObject>

class QGuiApplication;
class QQmlApplicationEngine;
class QTranslator;
class QQmlContext;

class Bluetooth;

class OneControl
    : public QObject
{
    Q_OBJECT

public:
    inline static OneControl* I() { if( s_pInstance == NULL ) s_pInstance = new OneControl(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }
    static void signalHandler(int signal);

    void init(QGuiApplication *app);

    void setLocale(QString locale);

    Q_INVOKABLE Bluetooth* bluetooth() { return m_bluetooth; }

signals:

public slots:
    void deleteMe() { OneControl::destroyI(); }

private:
    explicit OneControl(QObject *parent = 0);
    ~OneControl();

    static OneControl *s_pInstance;

    void initInterface();
    void initLocale();
    void initEngine();

    QQmlApplicationEngine *m_engine;
    QGuiApplication *m_app;
    QQmlContext     *m_context;
    QTranslator     *m_translator;

    Bluetooth       *m_bluetooth;
};

#endif // ONECONTROL_H
