#include "bluetoothmultiplexer.h"

#include <QTcpSocket>
#include <QBluetoothSocket>
#include <QDataStream>

#include "multiplexerhandler.h"

BluetoothMultiplexer::BluetoothMultiplexer(const QString &address, const QString &service_uuid, QObject *parent)
    : QObject(parent)
    , m_socket(NULL)
    , m_connections()
    , m_connections_nextid(1)
    , m_buffer_in_required(0)
    , m_buffer_in_data()
    , m_buffer_in_version(0)
    , m_buffer_in_type(0)
    , m_buffer_out_data()
{
    qDebug() << this << "Creating";

    m_buffer_out_data.clear();

    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);

    connect(m_socket, &QBluetoothSocket::connected, this, &BluetoothMultiplexer::_connected);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &BluetoothMultiplexer::_disconnected);
    connect(m_socket, &QBluetoothSocket::readyRead, this, &BluetoothMultiplexer::_read);
    connect(m_socket, &QBluetoothSocket::stateChanged, this, &BluetoothMultiplexer::_stateChanged);
    connect(m_socket, static_cast<void(QBluetoothSocket::*)(QBluetoothSocket::SocketError)>(&QBluetoothSocket::error),
            this, &BluetoothMultiplexer::_error);

    processNext(2, &BluetoothMultiplexer::_processInHeader);

    m_socket->connectToService(QBluetoothAddress(address), QBluetoothUuid(service_uuid));
}

void BluetoothMultiplexer::writeGetAvailableServices()
{
    write(TYPE_GETSERVICES, QByteArray());
}

void BluetoothMultiplexer::writeSetService(const quint8 id, QString name)
{
    QByteArray out;
    QDataStream stream(&out, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    stream << id << static_cast<quint8>(name.length());
    out.append(name.toLocal8Bit());

    write(TYPE_SETSERVICE, out);
}

void BluetoothMultiplexer::writeServiceData(const PacketType type, const quint8 id, const QByteArray &data)
{
    QByteArray out;
    if( type == TYPE_QCOMPRESS )
        out = qCompress(data);
    else
        out = data;

    QByteArray header;
    QDataStream stream(&header, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    stream << static_cast<quint8>(id);         // Mux channel id
    stream << static_cast<qint32>(out.size()); // Size of data

    out.prepend(header);

    write(type, out);
}

void BluetoothMultiplexer::writeSocketClose(quint8 id)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    stream << id;

    write(TYPE_SOCKET_CLOSE, data);
}

void BluetoothMultiplexer::write(const PacketType type, const QByteArray &data)
{
    QDataStream stream(&m_buffer_out_data, QIODevice::Append);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    stream << static_cast<quint8>(0);          // Version
    stream << static_cast<quint8>(type);       // Type

    //qDebug() << this << "Writing type:" << type << "size:" << data.size();

    m_buffer_out_data.append(data);
    _write();
}

void BluetoothMultiplexer::createTcpConnection(const QString &service_name, QTcpSocket *client_socket)
{
    quint8 id = m_connections_nextid;
    while( m_connections.contains(id) )
        id++;
    m_connections_nextid = id+1;

    writeSetService(id, service_name);

    m_connections.insert(id, new MultiplexerHandler(id, client_socket, this, this));
    connect(client_socket, &QAbstractSocket::disconnected, m_connections[id], &QObject::deleteLater);

    qDebug() << this << "created connection id:" << id << m_connections.keys();
}

void BluetoothMultiplexer::removeTcpConnection(quint8 id)
{
    qDebug() << this << "Removing connection" << id;
    m_connections.remove(id);
}

void BluetoothMultiplexer::processNext(qint32 required_bytes, void (BluetoothMultiplexer::*function)())
{
    if( m_buffer_in_required > 0 )
        m_buffer_in_data.remove(0, m_buffer_in_required);
    m_buffer_in_required = required_bytes; // Awaiting for data header
    disconnect(this, &BluetoothMultiplexer::_bufferInReady, 0, 0);
    connect(this, &BluetoothMultiplexer::_bufferInReady, function);

    _read();
}

void BluetoothMultiplexer::_read()
{
    m_buffer_in_data.append(m_socket->read(m_buffer_in_required));
    //qDebug() << this << "Was read" << m_buffer_in_data.toHex();
    if( m_buffer_in_data.size() >= m_buffer_in_required )
        emit _bufferInReady();
}

void BluetoothMultiplexer::_write()
{
    disconnect(m_socket, &QBluetoothSocket::bytesWritten, this, &BluetoothMultiplexer::_write);
    if( m_buffer_out_data.size() > 0 ) {
        //qDebug() << this << "Writing data len:" << m_buffer_out_data.size();
        qint64 written = m_socket->write(m_buffer_out_data);
        //qDebug() << this << "Written:" << m_buffer_out_data.left(static_cast<int>(written)).toHex();
        m_buffer_out_data.remove(0, static_cast<int>(written));
        if( m_buffer_out_data.size() > 0 )
            connect(m_socket, &QBluetoothSocket::bytesWritten, this, &BluetoothMultiplexer::_write);
    }
}

void BluetoothMultiplexer::_processInHeader()
{
    QDataStream stream(&m_buffer_in_data, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    stream >> m_buffer_in_version >> m_buffer_in_type;

    switch( m_buffer_in_type ) {
        case TYPE_DATA:
            //qDebug() << this << "Receiving data";
            processNext(5, &BluetoothMultiplexer::_processInDataHeader);
            break;
        case TYPE_QCOMPRESS:
            //qDebug() << this << "Receiving compressed data";
            processNext(5, &BluetoothMultiplexer::_processInDataHeader);
            break;
        case TYPE_GETSERVICES:
            qDebug() << this << "Receiving services list";
            processNext(4, &BluetoothMultiplexer::_processInServiceHeader);
            break;
        case TYPE_GETSERVICE:
            qDebug() << this << "Receiving service name";
            processNext(1, &BluetoothMultiplexer::_processInServiceHeader);
            break;
        case TYPE_SOCKET_CLOSE:
            qDebug() << this << "Received close connection";
            processNext(1, &BluetoothMultiplexer::_processSocket);
            break;
    }
}

void BluetoothMultiplexer::_processInDataHeader()
{
    QDataStream stream(&m_buffer_in_data, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    qint32 length;
    stream >> m_buffer_in_data_id >> length;

    processNext(length, &BluetoothMultiplexer::_processInData);
}

void BluetoothMultiplexer::_processInData()
{
    QByteArray out;
    if( m_buffer_in_type == TYPE_QCOMPRESS )
        out = qUncompress(m_buffer_in_data.left(m_buffer_in_required));
    else
        out = m_buffer_in_data.left(m_buffer_in_required);

    if( m_connections.contains(m_buffer_in_data_id) )
        m_connections[m_buffer_in_data_id]->clientWrite(out);
    else {
        qWarning() << this << "Unable to find connection id:" << m_buffer_in_data_id << m_connections.keys();
        writeSocketClose(m_buffer_in_data_id);
    }

    processNext(2, &BluetoothMultiplexer::_processInHeader);
}

void BluetoothMultiplexer::_processInServiceHeader()
{
    QDataStream stream(&m_buffer_in_data, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    qint32 length;
    if( m_buffer_in_type == TYPE_GETSERVICES )
        stream >> length;
    else {
        quint8 tmp;
        stream >> tmp;
        length = tmp+1; // Plus byte for id
    }

    processNext(length, &BluetoothMultiplexer::_processInService);
}

void BluetoothMultiplexer::_processInService()
{
    if( m_buffer_in_type == TYPE_GETSERVICES ) {
        QStringList out;
        QList<QByteArray> services = m_buffer_in_data.left(m_buffer_in_required).split(0x00);
        foreach(QByteArray svc, services)
            out.append(QString(svc));

        emit availableServices(out);
    } else {
        QDataStream stream(&m_buffer_in_data, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::BigEndian);
        stream.setVersion(QDataStream::Qt_5_6);

        quint8 id;
        stream >> id;

        emit currentService(id, QString(m_buffer_in_data.mid(1, m_buffer_in_required)));
    }

    processNext(2, &BluetoothMultiplexer::_processInHeader);
}

void BluetoothMultiplexer::_processSocket()
{
    QDataStream stream(&m_buffer_in_data, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream.setVersion(QDataStream::Qt_5_6);

    quint8 id;
    stream >> id;

    // Close socket connection
    if( m_connections.contains(id) ) {
        delete m_connections[id];
        removeTcpConnection(id);
    }

    processNext(2, &BluetoothMultiplexer::_processInHeader);
}

void BluetoothMultiplexer::_connected()
{
    qDebug() << this << "connected";
    writeGetAvailableServices();
    emit stateChanged(true);
}

void BluetoothMultiplexer::_disconnected()
{
    qDebug() << this << "disconnected";
    availableServices(QStringList());
    emit stateChanged(false);
}

void BluetoothMultiplexer::_stateChanged()
{
    qDebug() << this << "stateChanged" << m_socket->state();
}

void BluetoothMultiplexer::_error()
{
    qWarning() << this << "Socket error: " << m_socket->error() << m_socket->errorString();
}
