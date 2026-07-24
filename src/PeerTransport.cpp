#include "PeerTransport.h"
#include "MessageCodec.h"

#include <QDateTime>
#include <QHostAddress>
#include <QJsonObject>
#include <QUuid>

PeerTransport::PeerTransport(QObject *parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &PeerTransport::acceptPendingConnection);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, &PeerTransport::sendHeartbeat);
    m_heartbeatTimer.setInterval(5000);
}

bool PeerTransport::startListening(quint16 port)
{
    stopListening();
    if (!m_server.listen(QHostAddress::AnyIPv4, port)) {
        emit errorOccurred(QStringLiteral("监听失败：%1").arg(m_server.errorString()));
        return false;
    }
    emit listeningChanged(true, m_server.serverPort());
    emit logMessage(QStringLiteral("正在监听 0.0.0.0:%1").arg(m_server.serverPort()));
    return true;
}

void PeerTransport::stopListening()
{
    if (!m_server.isListening()) return;
    m_server.close();
    emit listeningChanged(false, 0);
    emit logMessage(QStringLiteral("已停止监听"));
}

void PeerTransport::connectToPeer(const QString &host, quint16 port)
{
    resetSocket();
    auto *socket = new QTcpSocket(this);
    adoptSocket(socket);
    emit logMessage(QStringLiteral("正在连接 %1:%2").arg(host).arg(port));
    socket->connectToHost(host, port);
}

void PeerTransport::disconnectPeer()
{
    if (!m_socket) return;
    m_socket->disconnectFromHost();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->waitForDisconnected(300);
    }
    resetSocket();
}

bool PeerTransport::sendText(const QString &text)
{
    if (!isConnected() || text.trimmed().isEmpty()) return false;

    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QJsonObject object{
        {QStringLiteral("type"), QStringLiteral("text")},
        {QStringLiteral("id"), id},
        {QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()},
        {QStringLiteral("content"), text}
    };
    sendObject(object);
    return true;
}

bool PeerTransport::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

bool PeerTransport::isListening() const
{
    return m_server.isListening();
}

QString PeerTransport::peerDescription() const
{
    if (!isConnected()) return QStringLiteral("未连接");
    return QStringLiteral("%1:%2").arg(m_socket->peerAddress().toString()).arg(m_socket->peerPort());
}

void PeerTransport::acceptPendingConnection()
{
    while (m_server.hasPendingConnections()) {
        QTcpSocket *incoming = m_server.nextPendingConnection();
        if (isConnected()) {
            incoming->disconnectFromHost();
            incoming->deleteLater();
            emit logMessage(QStringLiteral("拒绝额外连接：当前仅允许一个对端"));
            continue;
        }
        resetSocket();
        adoptSocket(incoming);
        handleConnected();
    }
}

void PeerTransport::readSocketData()
{
    if (!m_socket) return;
    m_receiveBuffer.append(m_socket->readAll());

    while (true) {
        QJsonObject object;
        QString error;
        const bool decoded = MessageCodec::tryDecode(m_receiveBuffer, object, error);
        if (!error.isEmpty()) {
            emit errorOccurred(error);
            continue;
        }
        if (!decoded) break;
        processObject(object);
    }
}

void PeerTransport::handleConnected()
{
    if (!m_socket) return;
    m_heartbeatTimer.start();
    emit connectionChanged(true, peerDescription());
    emit logMessage(QStringLiteral("连接已建立：%1").arg(peerDescription()));

    sendObject(QJsonObject{
        {QStringLiteral("type"), QStringLiteral("hello")},
        {QStringLiteral("name"), QStringLiteral("BIT-P2P-Linux")},
        {QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()}
    });
}

void PeerTransport::handleDisconnected()
{
    m_heartbeatTimer.stop();
    emit connectionChanged(false, QStringLiteral("未连接"));
    emit logMessage(QStringLiteral("对端连接已断开"));
}

void PeerTransport::sendHeartbeat()
{
    if (!isConnected()) return;
    sendObject(QJsonObject{
        {QStringLiteral("type"), QStringLiteral("heartbeat")},
        {QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()}
    });
}

void PeerTransport::adoptSocket(QTcpSocket *socket)
{
    m_socket = socket;
    connect(m_socket, &QTcpSocket::readyRead, this, &PeerTransport::readSocketData);
    connect(m_socket, &QTcpSocket::connected, this, &PeerTransport::handleConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &PeerTransport::handleDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        if (m_socket) emit errorOccurred(QStringLiteral("网络错误：%1").arg(m_socket->errorString()));
    });
}

void PeerTransport::sendObject(const QJsonObject &object)
{
    if (!isConnected()) return;
    const QByteArray frame = MessageCodec::encode(object);
    const qint64 written = m_socket->write(frame);
    if (written < 0) emit errorOccurred(QStringLiteral("发送失败：%1").arg(m_socket->errorString()));
}

void PeerTransport::processObject(const QJsonObject &object)
{
    const QString type = object.value(QStringLiteral("type")).toString();
    if (type == QStringLiteral("text")) {
        const QString id = object.value(QStringLiteral("id")).toString();
        const QString content = object.value(QStringLiteral("content")).toString();
        const qint64 timestamp = object.value(QStringLiteral("timestamp")).toVariant().toLongLong();
        emit textReceived(content, id, QDateTime::fromMSecsSinceEpoch(timestamp, Qt::UTC).toLocalTime());
        sendObject(QJsonObject{
            {QStringLiteral("type"), QStringLiteral("ack")},
            {QStringLiteral("id"), id},
            {QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toMSecsSinceEpoch()}
        });
    } else if (type == QStringLiteral("ack")) {
        emit deliveryReceived(object.value(QStringLiteral("id")).toString());
    } else if (type == QStringLiteral("hello")) {
        emit logMessage(QStringLiteral("对端标识：%1").arg(object.value(QStringLiteral("name")).toString()));
    }
}

void PeerTransport::resetSocket()
{
    m_heartbeatTimer.stop();
    m_receiveBuffer.clear();
    if (!m_socket) return;
    m_socket->disconnect(this);
    m_socket->abort();
    m_socket->deleteLater();
    m_socket = nullptr;
    emit connectionChanged(false, QStringLiteral("未连接"));
}
