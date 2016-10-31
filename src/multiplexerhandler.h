#ifndef MULTIPLEXERHANDLER_H
#define MULTIPLEXERHANDLER_H

#include <QObject>

class QAbstractSocket;
class BluetoothMultiplexer;

class MultiplexerHandler
    : public QObject
{
    Q_OBJECT

public:
    explicit MultiplexerHandler(quint8 id, QAbstractSocket *client, BluetoothMultiplexer *mux, QObject *parent = 0);
    ~MultiplexerHandler();

    void clientWrite(const QByteArray &data);

private:
    quint8 m_id;
    QAbstractSocket *m_client;
    BluetoothMultiplexer *m_mux;

private slots:
    void clientRead();
    void close();
    void clientError();
    void stateChanged();
};

#endif // MULTIPLEXERHANDLER_H
