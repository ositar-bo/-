#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

class PeerTransport final : public QObject
{
    Q_OBJECT
public:
    explicit PeerTransport(QObject *parent = nullptr);

    bool startListening(quint16 port);
    void stopListening();
    void connectToPeer(const QString &host, quint16 port);
    void disconnectPeer();
    bool sendText(const QString &text);
    bool isConnected() const;
    bool isListening() const;
    QString peerDescription() const;

signals:
    void listeningChanged(bool listening, quint16 port);
    void connectionChanged(bool connected, const QString &description);
    void textReceived(const QString &text, const QString &messageId, const QDateTime &time);
    void deliveryReceived(const QString &messageId);
    void logMessage(const QString &message);
    void errorOccurred(const QString &message);

private slots:
    void acceptPendingConnection();
    void readSocketData();
    void handleConnected();
    void handleDisconnected();
    void sendHeartbeat();

private:
    void adoptSocket(QTcpSocket *socket);
    void sendObject(const QJsonObject &object);
    void processObject(const QJsonObject &object);
    void resetSocket();

    QTcpServer m_server;
    QTcpSocket *m_socket = nullptr;
    QByteArray m_receiveBuffer;
    QTimer m_heartbeatTimer;
};
