#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include <QObject>
#include <QHostAddress>

class QTcpServer;
class BluetoothMultiplexer;

class TCPListener
    : public QObject
{
    Q_OBJECT

public:
    explicit TCPListener(const QString &service_name, BluetoothMultiplexer *mux, QObject *parent = 0);
    ~TCPListener();

    quint16 getPort();
    QHostAddress getAddress();

private slots:
    void _newTcpConnection();

private:
    QTcpServer *m_tcpserver;
    QString m_service_name;
    BluetoothMultiplexer *m_mux;
};

#endif // TCPLISTENER_H
