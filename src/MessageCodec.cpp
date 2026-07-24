#include "MessageCodec.h"

#include <QDataStream>
#include <QJsonDocument>

namespace {
constexpr quint32 kMaxFrameSize = 4U * 1024U * 1024U;
}

QByteArray MessageCodec::encode(const QJsonObject &object)
{
    const QByteArray payload = QJsonDocument(object).toJson(QJsonDocument::Compact);
    QByteArray frame;
    frame.reserve(static_cast<int>(sizeof(quint32)) + payload.size());

    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(payload.size());
    frame.append(payload);
    return frame;
}

bool MessageCodec::tryDecode(QByteArray &buffer, QJsonObject &object, QString &error)
{
    error.clear();
    if (buffer.size() < static_cast<int>(sizeof(quint32))) {
        return false;
    }

    QDataStream stream(buffer.left(static_cast<int>(sizeof(quint32))));
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 payloadSize = 0;
    stream >> payloadSize;

    if (payloadSize == 0 || payloadSize > kMaxFrameSize) {
        error = QStringLiteral("非法消息长度：%1").arg(payloadSize);
        buffer.clear();
        return false;
    }

    const qsizetype frameSize = static_cast<qsizetype>(sizeof(quint32)) + payloadSize;
    if (buffer.size() < frameSize) {
        return false;
    }

    const QByteArray payload = buffer.mid(static_cast<int>(sizeof(quint32)), static_cast<int>(payloadSize));
    buffer.remove(0, static_cast<int>(frameSize));

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        error = QStringLiteral("JSON解析失败：%1").arg(parseError.errorString());
        return false;
    }

    object = doc.object();
    return true;
}
