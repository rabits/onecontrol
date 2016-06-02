#include "onecontrol.h"

#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QtQml/QQmlContext>
#include <QTranslator>
#include <QScreen>
#include <QDir>
#include <QStandardPaths>

#include <signal.h>

#include "settings.h"

OneControl* OneControl::s_pInstance = NULL;

OneControl::OneControl(QObject *parent)
    : QObject(parent)
    , m_engine(NULL)
    , m_app(NULL)
    , m_translator(NULL)
{
    qDebug("OneControl v%s", PROJECT_VERSION);

    signal(SIGINT, OneControl::signalHandler);

    QCoreApplication::setOrganizationName("rabits.org");
    QCoreApplication::setOrganizationDomain("rabits.org");
    QCoreApplication::setApplicationName("OneControl");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    // Application locale
    Settings::I()->setDefault("onecontrol/locale", QLocale::system().name());
}

OneControl::~OneControl()
{
    Settings::destroyI();
    delete m_translator;
    delete m_engine;
    qDebug("Destroy OneControl");
}

void OneControl::signalHandler(int signal)
{
    qDebug() << "Received signal:" << signal;
}

void OneControl::init(QGuiApplication *app)
{
    qDebug("Init OneControl");

    m_app = app;
    connect(app, SIGNAL(aboutToQuit()), this, SLOT(deleteMe()));

    initInterface();
    initLocale();
    initEngine();

}

void OneControl::setLocale(QString locale)
{
    qDebug() << "Set locale to" << locale;

    if( ! m_translator->load("tr_" + locale, ":/") )
    {
        m_translator->load("tr_en", ":/");
        Settings::I()->setting("onecontrol/locale", "en");
    }
}

void OneControl::initInterface()
{
    qDebug("Init OneControl interface");

    m_engine = new QQmlApplicationEngine(this);
    m_context = m_engine->rootContext();

    m_context->setContextProperty("app", this);
    m_context->setContextProperty("cfg", Settings::I());
    m_context->setContextProperty("ss", QGuiApplication::primaryScreen()->physicalDotsPerInch() * QGuiApplication::primaryScreen()->devicePixelRatio() / 100);
}

void OneControl::initLocale()
{
    qDebug("Init OneControl locale");

    m_translator = new QTranslator(this);
    setLocale(Settings::I()->setting("onecontrol/locale").toString());

    m_app->installTranslator(m_translator);
}

void OneControl::initEngine()
{
    qDebug("Init OneControl qml interface");

    m_engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
}
