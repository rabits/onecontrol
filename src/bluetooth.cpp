#include "bluetooth.h"
#include "tcplistener.h"

#include <QDebug>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QCoreApplication>

#include "settings.h"
#include "bluetoothmultiplexer.h"

Bluetooth::Bluetooth(QObject *parent)
    : QObject(parent)
    , m_device(NULL)
    , m_discovery_agent(NULL)
    , m_address("")
    , m_service_name("")
    , m_service_uuid(NULL)
    , m_mux(NULL)
{
    qDebug() << this << "Creating";
    m_device = new QBluetoothLocalDevice(this);
    m_service_name = Settings::I()->setting("bluetooth/service_onebutton").toString();
    m_service_uuid = new QBluetoothUuid(Settings::I()->setting("bluetooth/service_onebutton_uuid").toString());

    if( m_device->isValid() ) {
        m_device->powerOn();

        m_discovery_agent = new QBluetoothServiceDiscoveryAgent(this);
        connect(m_discovery_agent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    }
}

Bluetooth::~Bluetooth()
{
    qDebug() << this << "Destroying";
    delete m_discovery_agent;
    delete m_service_uuid;
    delete m_device;
}

QString Bluetooth::getName()
{
    return m_device->name();
}

void Bluetooth::discoveryStart()
{
    qDebug() << this << "discoveryStart";
    m_discovery_agent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
}

void Bluetooth::discoveryStop()
{
    qDebug() << this << "discoveryStop";
    m_discovery_agent->stop();
}

void Bluetooth::connectTo(const QString &address)
{
    delete m_mux;
    m_mux = new BluetoothMultiplexer(address, Settings::I()->setting("bluetooth/service_onebutton_uuid").toString());
    connect(m_mux, &BluetoothMultiplexer::stateChanged, this, &Bluetooth::stateChanged);
    connect(m_mux, &BluetoothMultiplexer::availableServices, this, &Bluetooth::setAvailableServices);
}

QString Bluetooth::getServiceAddress(const QString &service_name)
{
    if( ! m_services_available.contains(service_name) ) {
        qWarning() << this << "Unable to find required mux service" << service_name << "in available services" << m_services_available;
        return "";
    }
    if( ! m_listeners.contains(service_name) )
        m_listeners[service_name] = new TCPListener(service_name, m_mux, this);

    QString address_port = m_listeners[service_name]->getAddress().toString();
    address_port.append(":").append(QString::number(m_listeners[service_name]->getPort()));

    qDebug() << this << "Address:" << address_port;

    return address_port;
}

void Bluetooth::serviceDiscovered(const QBluetoothServiceInfo &service)
{
    qDebug() << this << "Discovered device" << service.device().name() << "Service:" << service.serviceName()
             << "UUID:" << service.serviceUuid();

    if( service.serviceUuid() == *m_service_uuid || service.serviceName() == m_service_name )
        emit deviceFound(service.device().name(), service.device().address().toString());
}

void Bluetooth::setAvailableServices(QStringList services)
{
    qDebug() << this << "Updating available services:" << services;
    m_services_available = services;
    emit availableServices(m_services_available);
}
