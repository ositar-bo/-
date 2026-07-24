#pragma once

#include <QByteArray>
#include <QJsonObject>

namespace MessageCodec {
QByteArray encode(const QJsonObject &object);
bool tryDecode(QByteArray &buffer, QJsonObject &object, QString &error);
}
