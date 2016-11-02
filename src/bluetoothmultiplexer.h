#ifndef BLUETOOTHMULTIPLEXER_H
#define BLUETOOTHMULTIPLEXER_H

#include <QObject>
#include <QMap>

class QTcpSocket;
class QBluetoothSocket;

class MultiplexerHandler;

class BluetoothMultiplexer
    : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothMultiplexer(const QString &address, const QString &service_uuid, QObject *parent = 0);

    enum PacketType {
        TYPE_DATA = 0x00,         // Plain data
        TYPE_ZLIB = 0x01,         // ZLIB Compressed data
        TYPE_QCOMPRESS = 0x02,    // qCompress compressed data

        TYPE_GETSERVICES = 0x10,  // Get services list
        TYPE_GETSERVICE = 0x11,   // Get service
        TYPE_SETSERVICE = 0x12,   // Set service

        TYPE_SOCKET_CLOSE = 0xff  // Socket closed
    };

    void writeGetAvailableServices();
    void writeSetService(const quint8 id, QString name);
    void writeServiceData(const PacketType type, const quint8 id, const QByteArray &data);
    void writeSocketClose(quint8 id);
    void write(const PacketType type, const QByteArray &data);

    void createTcpConnection(const QString &service_name, QTcpSocket *client_socket);
    void removeTcpConnection(quint8 id);

private:
    QBluetoothSocket *m_socket;

    QMap<quint8, MultiplexerHandler*> m_connections;
    quint8 m_connections_nextid;

    qint32     m_buffer_in_required;
    QByteArray m_buffer_in_data;
    quint8     m_buffer_in_version;
    quint8     m_buffer_in_type;
    quint8     m_buffer_in_data_id;

    QByteArray m_buffer_out_data;

    void processNext(qint32 required_bytes, void (BluetoothMultiplexer::*function)());

signals:
    void availableServices(QStringList services);
    void currentService(quint8 id, QString service);
    void stateChanged(bool connected);

    void _bufferInReady();

private slots:
    void _read();
    void _write();
    void _connected();
    void _disconnected();
    void _stateChanged();
    void _error();

    void _processInHeader();
    void _processInDataHeader();
    void _processInData();

    void _processInServiceHeader();
    void _processInService();

    void _processSocket();
};

#endif // BLUETOOTHMULTIPLEXOR_H
