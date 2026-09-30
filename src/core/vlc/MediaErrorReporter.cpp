#include "MediaErrorReporter.h"

#include <QDateTime>

MediaErrorReporter::MediaErrorReporter(QObject* parent)
    : QObject(parent)
{
}

void MediaErrorReporter::reportOpenFailure(const QString& path, const QString& reason)
{
    m_lastError = QStringLiteral("无法打开视频:\n%1\n原因:\n%2\n时间:%3")
        .arg(path)
        .arg(reason)
        .arg(QDateTime::currentDateTime().toString(Qt::ISODate));

    emit errorReported(m_lastError);
}
