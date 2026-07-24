#include "MainWindow.h"
#include "PeerTransport.h"

#include <QApplication>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QStyle>
#include <QSplitter>
#include <QStatusBar>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_transport(new PeerTransport(this)),
      m_player(new QMediaPlayer(this))
{
    buildUi();
    applyStyle();

    connect(m_transport, &PeerTransport::connectionChanged, this, &MainWindow::setConnectedUi);
    connect(m_transport, &PeerTransport::textReceived, this,
            [this](const QString &text, const QString &, const QDateTime &time) { addMessage(text, false, time); });
    connect(m_transport, &PeerTransport::logMessage, this, &MainWindow::appendSystemLog);
    connect(m_transport, &PeerTransport::errorOccurred, this, [this](const QString &message) {
        appendSystemLog(QStringLiteral("错误：%1").arg(message));
        statusBar()->showMessage(message, 5000);
    });

    connect(m_connectButton, &QPushButton::clicked, this, [this] {
        if (m_transport->isConnected()) {
            m_transport->disconnectPeer();
            return;
        }
        m_transport->connectToPeer(m_ipEdit->text().trimmed(), configuredPort());
    });

    connect(m_listenButton, &QPushButton::clicked, this, [this] {
        if (m_transport->isListening()) {
            m_transport->stopListening();
            m_listenButton->setText(QStringLiteral("开始监听"));
        } else if (m_transport->startListening(configuredPort())) {
            m_listenButton->setText(QStringLiteral("停止监听"));
        }
    });

    auto sendCurrentMessage = [this] {
        const QString text = m_messageEdit->text().trimmed();
        if (text.isEmpty()) return;
        if (!m_transport->sendText(text)) {
            statusBar()->showMessage(QStringLiteral("尚未连接，消息未发送"), 3000);
            return;
        }
        addMessage(text, true);
        m_messageEdit->clear();
    };
    connect(m_sendButton, &QPushButton::clicked, this, sendCurrentMessage);
    connect(m_messageEdit, &QLineEdit::returnPressed, this, sendCurrentMessage);

    m_player->setVideoOutput(m_videoWidget);
    connect(m_playButton, &QPushButton::clicked, this, [this] {
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->stop();
            m_playButton->setText(QStringLiteral("播放视频流"));
            m_videoStatus->setText(QStringLiteral("● 已停止"));
            return;
        }
        const QUrl url = QUrl::fromUserInput(m_streamEdit->text().trimmed());
        if (!url.isValid()) {
            QMessageBox::warning(this, QStringLiteral("地址错误"), QStringLiteral("请输入有效的视频流地址。"));
            return;
        }
        m_player->setSource(url);
        m_player->play();
        m_playButton->setText(QStringLiteral("停止播放"));
        m_videoStatus->setText(QStringLiteral("● 正在连接视频流"));
    });
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::BufferedMedia || status == QMediaPlayer::LoadedMedia)
            m_videoStatus->setText(QStringLiteral("● 视频接收中"));
        else if (status == QMediaPlayer::InvalidMedia)
            m_videoStatus->setText(QStringLiteral("● 视频流不可用"));
    });
    connect(m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &error) { appendSystemLog(QStringLiteral("视频错误：%1").arg(error)); });

    resize(1360, 820);
    setMinimumSize(1050, 680);
}

void MainWindow::buildUi()
{
    #ifdef Q_OS_MACOS
    setWindowTitle(QStringLiteral("北京理工大学 - 点对点通信系统（macOS）"));
#else
    setWindowTitle(QStringLiteral("北京理工大学 - 点对点通信系统（Linux）"));
#endif

    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(18, 14, 18, 14);
    root->setSpacing(12);

    auto *header = new QHBoxLayout;
    auto *brand = new QVBoxLayout;
    auto *title = new QLabel(QStringLiteral("北 京 理 工 大 学"));
    title->setObjectName(QStringLiteral("brandTitle"));
    auto *subtitle = new QLabel(QStringLiteral("BEIJING INSTITUTE OF TECHNOLOGY · 点对点通信系统"));
    subtitle->setObjectName(QStringLiteral("brandSubtitle"));
    brand->addWidget(title);
    brand->addWidget(subtitle);
    header->addLayout(brand);
    header->addStretch();
    m_statusDot = new QLabel(QStringLiteral("●"));
    m_statusDot->setObjectName(QStringLiteral("statusOffline"));
    m_statusLabel = new QLabel(QStringLiteral("未连接"));
    header->addWidget(m_statusDot);
    header->addWidget(m_statusLabel);
    root->addLayout(header);

    auto *splitter = new QSplitter(Qt::Horizontal);

    auto *leftPanel = new QWidget;
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 8, 0);

    auto *connectionBox = new QGroupBox(QStringLiteral("连接设置"));
    auto *connectionLayout = new QFormLayout(connectionBox);
    m_ipEdit = new QLineEdit(QStringLiteral("127.0.0.1"));
    m_portEdit = new QLineEdit(QStringLiteral("5000"));
    m_portEdit->setMaximumWidth(120);
    connectionLayout->addRow(QStringLiteral("目标 IP"), m_ipEdit);
    connectionLayout->addRow(QStringLiteral("通信端口"), m_portEdit);
    auto *connectionButtons = new QHBoxLayout;
    m_listenButton = new QPushButton(QStringLiteral("开始监听"));
    m_connectButton = new QPushButton(QStringLiteral("连接对端"));
    m_connectButton->setObjectName(QStringLiteral("primaryButton"));
    connectionButtons->addWidget(m_listenButton);
    connectionButtons->addWidget(m_connectButton);
    connectionLayout->addRow(connectionButtons);
    m_linkInfo = new QLabel(QStringLiteral("等待建立点对点链路"));
    m_linkInfo->setWordWrap(true);
    m_linkInfo->setObjectName(QStringLiteral("hintLabel"));
    connectionLayout->addRow(m_linkInfo);
    leftLayout->addWidget(connectionBox);

    auto *chatBox = new QGroupBox(QStringLiteral("文本通信"));
    auto *chatLayout = new QVBoxLayout(chatBox);
    m_chatView = new QTextBrowser;
    m_chatView->setOpenExternalLinks(false);
    chatLayout->addWidget(m_chatView, 1);
    auto *inputLayout = new QHBoxLayout;
    m_messageEdit = new QLineEdit;
    m_messageEdit->setPlaceholderText(QStringLiteral("输入消息，按 Enter 发送"));
    m_sendButton = new QPushButton(QStringLiteral("发送"));
    m_sendButton->setObjectName(QStringLiteral("primaryButton"));
    inputLayout->addWidget(m_messageEdit, 1);
    inputLayout->addWidget(m_sendButton);
    chatLayout->addLayout(inputLayout);
    leftLayout->addWidget(chatBox, 1);

    auto *logBox = new QGroupBox(QStringLiteral("运行日志"));
    auto *logLayout = new QVBoxLayout(logBox);
    m_logView = new QTextBrowser;
    m_logView->setMaximumHeight(130);
    logLayout->addWidget(m_logView);
    leftLayout->addWidget(logBox);

    auto *rightPanel = new QWidget;
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(8, 0, 0, 0);
    auto *videoBox = new QGroupBox(QStringLiteral("视频接收"));
    auto *videoLayout = new QVBoxLayout(videoBox);
    m_videoWidget = new QVideoWidget;
    m_videoWidget->setMinimumSize(520, 390);
    videoLayout->addWidget(m_videoWidget, 1);
    auto *streamRow = new QHBoxLayout;
    m_streamEdit = new QLineEdit(QStringLiteral("rtsp://127.0.0.1:8554/live"));
    m_playButton = new QPushButton(QStringLiteral("播放视频流"));
    m_playButton->setObjectName(QStringLiteral("primaryButton"));
    streamRow->addWidget(m_streamEdit, 1);
    streamRow->addWidget(m_playButton);
    videoLayout->addLayout(streamRow);
    m_videoStatus = new QLabel(QStringLiteral("● 等待视频流"));
    m_videoStatus->setObjectName(QStringLiteral("videoStatus"));
    videoLayout->addWidget(m_videoStatus);
    rightLayout->addWidget(videoBox, 1);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 7);
    root->addWidget(splitter, 1);

    setCentralWidget(central);
    statusBar()->showMessage(QStringLiteral("系统就绪"));
}

void MainWindow::applyStyle()
{
    qApp->setStyleSheet(QStringLiteral(R"(
        QWidget { background:#0b120e; color:#e8eee9; font-family:"Noto Sans CJK SC","Microsoft YaHei",sans-serif; font-size:14px; }
        QMainWindow, QStatusBar { background:#080d0a; }
        #brandTitle { font-size:25px; font-weight:700; letter-spacing:5px; color:#f5f7f5; }
        #brandSubtitle { font-size:11px; color:#9aa89e; letter-spacing:1px; }
        #statusOffline { color:#778078; font-size:18px; }
        #statusOnline { color:#49c867; font-size:18px; }
        #hintLabel { color:#87b98f; padding:7px 0; }
        #videoStatus { color:#58c875; padding:4px; }
        QGroupBox { border:1px solid #26352b; border-radius:5px; margin-top:12px; padding-top:12px; font-weight:600; }
        QGroupBox::title { subcontrol-origin:margin; left:12px; padding:0 6px; color:#dfe7e1; }
        QLineEdit, QTextBrowser { background:#111a14; border:1px solid #2a382e; border-radius:4px; padding:9px; selection-background-color:#375f3c; }
        QLineEdit:focus, QTextBrowser:focus { border:1px solid #477c50; }
        QPushButton { background:#17231a; border:1px solid #324236; border-radius:4px; padding:9px 16px; min-height:20px; }
        QPushButton:hover { background:#213126; }
        QPushButton:pressed { background:#101a13; }
        QPushButton#primaryButton { background:#29442d; border-color:#3d6643; }
        QPushButton#primaryButton:hover { background:#35583b; }
        QVideoWidget { background:#050806; border:1px solid #27372c; }
        QSplitter::handle { background:#1d2a21; width:1px; }
        QScrollBar:vertical { background:#0d1510; width:10px; }
        QScrollBar::handle:vertical { background:#304034; border-radius:4px; min-height:30px; }
        QStatusBar { color:#849188; border-top:1px solid #1b281f; }
    )"));
}

void MainWindow::addMessage(const QString &text, bool local, const QDateTime &time)
{
    const QString side = local ? QStringLiteral("right") : QStringLiteral("left");
    const QString bg = local ? QStringLiteral("#243d29") : QStringLiteral("#18211b");
    const QString safe = text.toHtmlEscaped().replace(QStringLiteral("\n"), QStringLiteral("<br>"));
    const QString html = QStringLiteral(
        "<div align='%1' style='margin:8px 2px'>"
        "<table cellpadding='8' cellspacing='0' style='background:%2;border-radius:5px'><tr><td>"
        "<span style='color:#93a097;font-size:10px'>%3</span><br>"
        "<span style='color:#edf2ee'>%4</span>"
        "</td></tr></table></div>")
        .arg(side, bg, time.toString(QStringLiteral("HH:mm:ss")), safe);
    m_chatView->append(html);
    m_chatView->verticalScrollBar()->setValue(m_chatView->verticalScrollBar()->maximum());
}

void MainWindow::appendSystemLog(const QString &text)
{
    m_logView->append(QStringLiteral("[%1] %2").arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")), text.toHtmlEscaped()));
}

void MainWindow::setConnectedUi(bool connected, const QString &description)
{
    m_statusDot->setObjectName(connected ? QStringLiteral("statusOnline") : QStringLiteral("statusOffline"));
    m_statusDot->style()->unpolish(m_statusDot);
    m_statusDot->style()->polish(m_statusDot);
    m_statusLabel->setText(connected ? QStringLiteral("已连接") : QStringLiteral("未连接"));
    m_linkInfo->setText(connected ? QStringLiteral("点对点链路已建立：%1").arg(description)
                                  : QStringLiteral("等待建立点对点链路"));
    m_connectButton->setText(connected ? QStringLiteral("断开连接") : QStringLiteral("连接对端"));
    m_sendButton->setEnabled(connected);
}

quint16 MainWindow::configuredPort() const
{
    bool ok = false;
    const uint value = m_portEdit->text().toUInt(&ok);
    return ok && value > 0 && value <= 65535 ? static_cast<quint16>(value) : 5000;
}
