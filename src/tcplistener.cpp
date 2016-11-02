#include "tcplistener.h"
#include "bluetoothmultiplexer.h"

#include <QTcpServer>

TCPListener::TCPListener(const QString &service_name, BluetoothMultiplexer *mux, QObject *parent)
    : QObject(parent)
    , m_tcpserver(NULL)
    , m_service_name(service_name)
    , m_mux(mux)
{
    qDebug() << this << "Creating";

    m_tcpserver = new QTcpServer(this);
    connect(m_tcpserver, &QTcpServer::newConnection, this, &TCPListener::_newTcpConnection);
    m_tcpserver->listen(QHostAddress(QHostAddress::LocalHost));
}

TCPListener::~TCPListener()
{
    qDebug() << this << "Destroying";
    delete m_tcpserver;
}

quint16 TCPListener::getPort()
{
    return m_tcpserver->serverPort();
}

QHostAddress TCPListener::getAddress()
{
    return m_tcpserver->serverAddress();
}

void TCPListener::_newTcpConnection()
{
    qDebug() << this << "received new tcp connection";
    QTcpSocket *client_socket = m_tcpserver->nextPendingConnection();

    m_mux->createTcpConnection(m_service_name, client_socket);
}
