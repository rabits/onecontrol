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
#include <QUuid>
#include <QtWebView/QtWebView>

#include <signal.h>

#include "settings.h"
#include "bluetooth.h"

OneControl* OneControl::s_pInstance = NULL;

OneControl::OneControl(QObject *parent)
    : QObject(parent)
    , m_engine(NULL)
    , m_app(NULL)
    , m_translator(NULL)
    , m_bluetooth(NULL)
{
    qDebug() << this << "Creating";
    qDebug() << this << "version:" << PROJECT_VERSION;

    signal(SIGINT, OneControl::signalHandler);

    QCoreApplication::setOrganizationName("rabits.org");
    QCoreApplication::setOrganizationDomain("rabits.org");
    QCoreApplication::setApplicationName("OneControl");
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QtWebView::initialize();

    // Application locale
    Settings::I()->setDefault("onecontrol/locale", QLocale::system().name());

    // Bluetooth services
    Settings::I()->setDefault("bluetooth/service_onebutton", "OneButton");
    Settings::I()->setDefault("bluetooth/service_guitarix_rpc", "Guitarix RPC");
    Settings::I()->setDefault("bluetooth/service_guitarix_web", "Guitarix WEB");

    // Bluetooth UUID generation
    // Onebutton Bluetooth namespace ("OneButtonBlueTNS")
    QUuid onebutton_uuid("{4f6e6542-7574-746f-6e42-6c7565544e53}");
    Settings::I()->setting("bluetooth/service_onebutton_uuid",
                           QUuid::createUuidV5(onebutton_uuid,
                                               Settings::I()->setting("bluetooth/service_onebutton").toString()).toString());

    Settings::I()->setDefault("bluetooth/service_guitarix_web_port", 8003);
    Settings::I()->setDefault("bluetooth/service_guitarix_rpc_port", 8000);

    m_bluetooth = new Bluetooth(this);
}

OneControl::~OneControl()
{
    Settings::destroyI();
    delete m_translator;
    delete m_engine;
    delete m_bluetooth;
    qDebug() << this << "Destroying";
}

void OneControl::signalHandler(int signal)
{
    qDebug() << "Received signal:" << signal;
}

void OneControl::init(QGuiApplication *app)
{
    qDebug() << this << "init";

    m_app = app;
    connect(app, SIGNAL(aboutToQuit()), this, SLOT(deleteMe()));

    initInterface();
    initLocale();
    initEngine();
}

void OneControl::setLocale(QString locale)
{
    qDebug() << this << "Set locale to" << locale;

    if( ! m_translator->load("tr_" + locale, ":/") )
    {
        m_translator->load("tr_en", ":/");
        Settings::I()->setting("onecontrol/locale", "en");
    }
}

void OneControl::initInterface()
{
    qDebug() << this << "Init interface";

    m_engine = new QQmlApplicationEngine(this);
    m_context = m_engine->rootContext();

    m_context->setContextProperty("app", this);
    m_context->setContextProperty("cfg", Settings::I());
    m_context->setContextProperty("ss", QGuiApplication::primaryScreen()->physicalDotsPerInch() * QGuiApplication::primaryScreen()->devicePixelRatio() / 100);

    qmlRegisterUncreatableType<Bluetooth>("org.rabits.onecontrol", 1, 0, "Bluetooth", "External type");
}

void OneControl::initLocale()
{
    qDebug() << this << "Init locale";

    m_translator = new QTranslator(this);
    setLocale(Settings::I()->setting("onecontrol/locale").toString());

    m_app->installTranslator(m_translator);
}

void OneControl::initEngine()
{
    qDebug() << this << "Init qml interface";

    m_engine->load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
}
