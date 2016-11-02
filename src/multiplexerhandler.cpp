#include "multiplexerhandler.h"
#include "bluetoothmultiplexer.h"

#include <QAbstractSocket>

MultiplexerHandler::MultiplexerHandler(quint8 id, QAbstractSocket *client, BluetoothMultiplexer *mux, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_client(client)
    , m_mux(mux)
{
    qDebug() << this << "Creating";

    connect(m_client, &QAbstractSocket::disconnected, this, &MultiplexerHandler::close);
    connect(m_client, &QAbstractSocket::stateChanged, this, &MultiplexerHandler::stateChanged);
    connect(m_client, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
            this, &MultiplexerHandler::clientError);
    connect(m_client, &QAbstractSocket::readyRead, this, &MultiplexerHandler::clientRead);
    if( m_client->isReadable() )
        clientRead();
}

MultiplexerHandler::~MultiplexerHandler()
{
    qDebug() << this << "Destroying";
    delete m_client;
}

void MultiplexerHandler::clientRead()
{
    //qDebug() << this << "client->server";
    QByteArray data = m_client->readAll();
    if( data.size() == 0 ) {
        qDebug() << this << "  client read empty";
        return;
    }

    //qDebug() << this << "  client read done";
    m_mux->writeServiceData(BluetoothMultiplexer::TYPE_QCOMPRESS, m_id, data);
}

void MultiplexerHandler::clientWrite(const QByteArray &data)
{
    //qDebug() << this << "server->client";
    m_client->write(data);
    //qDebug() << this << "  client write done";
}

void MultiplexerHandler::close()
{
    qDebug() << this << "close";
    m_client->close();
    m_mux->writeSocketClose(m_id);
    m_mux->removeTcpConnection(m_id);
}

void MultiplexerHandler::clientError()
{
    qWarning() << this << "Client error: " << m_client->error() << m_client->errorString();
}

void MultiplexerHandler::stateChanged()
{
    qDebug() << this << "stateChanged" << m_client->state();
}
