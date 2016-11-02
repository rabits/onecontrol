#ifndef JSONRPC_H
#define JSONRPC_H

#include <QObject>
#include <QJsonDocument>
#include <QJSValue>
#include <QMap>

class QTcpSocket;

class JsonRPC
    : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString address_port READ getAddressPort WRITE setAddressPort NOTIFY addressPortChanged)
//    TODO: Need to implement
//    Q_PROPERTY(QString mode_continuous READ getModeContinuous WRITE setModeContinuous NOTIFY modeContinuousChanged)

public:
    explicit JsonRPC(const QString &address_port = "", QObject *parent = 0);
    ~JsonRPC();

    QString getAddressPort();
    void setAddressPort(const QString &address_port);
    bool getModeContinuous();
    void setModeContinuous(bool enabled);

    Q_INVOKABLE bool connectAddressPort();
    Q_INVOKABLE void requestCallback(const QString &method, const QJsonDocument &params, QJSValue js_callback);
    Q_INVOKABLE quint16 requestAsync(const QString &method, const QJsonDocument &params);

signals:
    void stateChanged(bool connected);
    void response(quint16 id, const QJsonValue result);

    void addressPortChanged();
    void modeContinuousChanged();

private:
    QString m_address;
    quint16 m_port;

    QTcpSocket *m_socket;
    quint16 m_id;

    QByteArray m_buffer_in_data;
    QByteArray m_buffer_out_data;

    bool m_mode_continuous;

    QMap<quint16, QJSValue> m_response_callbacks;

    void _parseResponse();
    void _parseResponseObject(const QJsonObject &resp);

    int _findJsonDocumentEnd(const QByteArray &json_data);
private slots:
    void _read();
    void _write();
    void _connected();
    void _disconnected();
    void _stateChanged();
    void _error();
};

#endif // JSONRPC_H
