#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QDateTime>

class QLabel;
class QLineEdit;
class QPushButton;
class QTextBrowser;
class QVideoWidget;
class PeerTransport;

class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void buildUi();
    void applyStyle();
    void addMessage(const QString &text, bool local, const QDateTime &time = QDateTime::currentDateTime());
    void appendSystemLog(const QString &text);
    void setConnectedUi(bool connected, const QString &description);
    quint16 configuredPort() const;

    PeerTransport *m_transport = nullptr;
    QMediaPlayer *m_player = nullptr;
    QVideoWidget *m_videoWidget = nullptr;

    QLabel *m_statusDot = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_linkInfo = nullptr;
    QLabel *m_videoStatus = nullptr;
    QLineEdit *m_ipEdit = nullptr;
    QLineEdit *m_portEdit = nullptr;
    QLineEdit *m_messageEdit = nullptr;
    QLineEdit *m_streamEdit = nullptr;
    QTextBrowser *m_chatView = nullptr;
    QTextBrowser *m_logView = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_listenButton = nullptr;
    QPushButton *m_sendButton = nullptr;
    QPushButton *m_playButton = nullptr;
};
