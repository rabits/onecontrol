#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QObject>
#include <QBluetoothServiceInfo>

class QBluetoothLocalDevice;
class QBluetoothServiceDiscoveryAgent;

class BluetoothMultiplexer;

class Bluetooth
    : public QObject
{
    Q_OBJECT

public:
    explicit Bluetooth(QObject *parent = 0);
    ~Bluetooth();

    Q_INVOKABLE QString getName();
    Q_INVOKABLE void discoveryStart();
    Q_INVOKABLE void discoveryStop();
    Q_INVOKABLE void connectTo(const QString &address);

signals:
    void deviceFound(const QString name, const QString address);
    void stateChanged(bool connected);

private:
    QBluetoothLocalDevice *m_device;
    QBluetoothServiceDiscoveryAgent *m_discovery_agent;
    QString m_address;

    QString         m_service_name;
    QBluetoothUuid *m_service_uuid;

    BluetoothMultiplexer *m_connection;

private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &device);
};

#endif // BLUETOOTH_H
