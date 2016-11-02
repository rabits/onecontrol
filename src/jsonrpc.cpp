#include "jsonrpc.h"
#include "onecontrol.h"

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>

JsonRPC::JsonRPC(const QString &address_port, QObject *parent)
    : QObject(parent)
    , m_address()
    , m_port(0)
    , m_socket(NULL)
    , m_id(0)
    , m_mode_continuous(false)
{
    qDebug() << this << "Creating";
    if( ! address_port.isEmpty() )
        setAddressPort(address_port);

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &JsonRPC::_connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &JsonRPC::_disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &JsonRPC::_read);
    connect(m_socket, static_cast<void(QTcpSocket::*)(QTcpSocket::SocketError)>(&QTcpSocket::error), this, &JsonRPC::_error);
}

JsonRPC::~JsonRPC()
{
    qDebug() << this << "Destroying";
    delete m_socket;
}

QString JsonRPC::getAddressPort()
{
    return m_address + ":" + QString::number(m_port);
}

void JsonRPC::setAddressPort(const QString &address_port)
{
    QStringList data = address_port.split(':');
    m_address = data.first();
    m_port = data.last().toUShort();

    m_socket->disconnectFromHost();
    emit addressPortChanged();
}

bool JsonRPC::getModeContinuous()
{
    return m_mode_continuous;
}

void JsonRPC::setModeContinuous(bool enabled)
{
    if( m_mode_continuous != enabled ) {
        m_mode_continuous = enabled;

        if( m_mode_continuous ) {
            m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, "1");
        } else {
            m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, "0");
            m_socket->disconnectFromHost();
        }

        emit modeContinuousChanged();
    }
}

bool JsonRPC::connectAddressPort()
{
    if( m_address.isEmpty() || m_port == 0 )
        return false;

    if( m_socket->state() != QAbstractSocket::ConnectedState )
        m_socket->connectToHost(m_address, m_port);

    if( ! m_socket->waitForConnected(5000) ) {
        qCritical() << this << "Could not connect to JSON RPC server: " << m_socket->errorString();
        return false;
    }

    return true;
}

void JsonRPC::requestCallback(const QString &method, const QJsonDocument &params, QJSValue js_callback)
{
    quint16 id = requestAsync(method, params);
    m_response_callbacks[id] = js_callback;
}

quint16 JsonRPC::requestAsync(const QString &method, const QJsonDocument &params)
{
    qDebug() << this << "Executing" << method << "with args" << params;
    if( ! connectAddressPort() ) {
        qCritical() << this << "Unable to connect to send request";
    }

    QJsonObject out;
    out["jsonrpc"] = "2.0";
    out["id"] = ++m_id;
    out["method"] = method;
    if( params.isArray() )
        out["params"] = params.array();
    else if( params.isObject() )
        out["params"] = params.object();

    m_buffer_out_data.append(QJsonDocument(out).toJson(QJsonDocument::Compact));
    qDebug() << this << m_buffer_out_data;

    if( m_mode_continuous )
        m_buffer_out_data.append('\n');

    _write();

    return m_id;
}

void JsonRPC::_parseResponse()
{
    while( !m_buffer_in_data.isEmpty() ) {
        int data_size = _findJsonDocumentEnd(m_buffer_in_data);

        if( data_size < 0 ) // Incomplete data?
            return;

        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(m_buffer_in_data.left(data_size + 1), &error);
        if( document.isEmpty() ) {
            if (error.error != QJsonParseError::NoError) {
                qDebug() << this << error.errorString();
            }

            break;
        }

        m_buffer_in_data.remove(0, data_size + 1);

        if( document.isArray() ){
            qDebug() << this << "Bulk support is disabled";
        } else if( document.isObject() ) {
            _parseResponseObject(document.object());
        } else
            qWarning() << this << "Wrong document" << document;
    }
}

void JsonRPC::_parseResponseObject(const QJsonObject &resp)
{
    if( resp.value("jsonrpc").toString() != "2.0" )
        qWarning() << this << "Invalid protocol version" << resp.value("jsonrpc").toString();

    if( resp.value("error").isObject() ) {
        qWarning() << this << "Response error:" << resp.value("error");
        return;
    }

    quint16 id = static_cast<quint16>(resp.value("id").toInt());

    if( m_response_callbacks.contains(id) ) {
        m_response_callbacks[id].call(QJSValueList { OneControl::I()->jsonValue2JSValue(resp.value("result")) });
        m_response_callbacks.remove(id);
    } else
        emit response(id, resp.value("result"));
}

int JsonRPC::_findJsonDocumentEnd(const QByteArray &json_data)
{
    // Func from qjsonrpc:

    const char* pos = json_data.constData();
    const char* end = pos + json_data.length();

    char block_start = 0;
    char block_end = 0;
    int index = 0;

    // Find the beginning of the JSON document and determine if it is an object or an array
    while( true ) {
        if( pos == end ) {
            return -1;
        } else if( *pos == '{' ) {
            block_start = '{';
            block_end = '}';
            break;
        } else if( *pos == '[' ) {
            block_start = '[';
            block_end = ']';
            break;
        }

        pos++;
        index++;
    }

    // Find the end of the JSON document
    pos++;
    index++;
    int depth = 1;
    bool in_string = false;
    while( depth > 0 && pos <= end ) {
        if (*pos == '\\') {
            pos += 2;
            index += 2;
            continue;
        } else if( *pos == '"' ) {
            in_string = !in_string;
        } else if( !in_string ) {
            if( *pos == block_start )
                depth++;
            else if( *pos == block_end )
                depth--;
        }

        pos++;
        index++;
    }

    // index-1 because we are one position ahead
    return depth == 0 ? index-1 : -1;
}

void JsonRPC::_read()
{
    m_buffer_in_data.append(m_socket->readAll());
    //qDebug() << this << "Was read" << m_buffer_in_data;
    _parseResponse();
}

void JsonRPC::_write()
{
    disconnect(m_socket, &QTcpSocket::bytesWritten, this, &JsonRPC::_write);
    if( m_buffer_out_data.size() > 0 ) {
        //qDebug() << this << "Writing data len:" << m_buffer_out_data.size();
        qint64 written = m_socket->write(m_buffer_out_data);
        //qDebug() << this << "Written:" << m_buffer_out_data.left(static_cast<int>(written));
        m_buffer_out_data.remove(0, static_cast<int>(written));
        if( m_buffer_out_data.size() > 0 )
            connect(m_socket, &QTcpSocket::bytesWritten, this, &JsonRPC::_write);
    }
}


void JsonRPC::_connected()
{
    qDebug() << this << "connected";
    emit stateChanged(true);
}

void JsonRPC::_disconnected()
{
    qDebug() << this << "disconnected";
    if( ! m_mode_continuous )
        _parseResponse();
    emit stateChanged(false);
}

void JsonRPC::_stateChanged()
{
    qDebug() << this << "stateChanged" << m_socket->state();
}

void JsonRPC::_error()
{
    qWarning() << this << "Socket error: " << m_socket->error() << m_socket->errorString();
}
