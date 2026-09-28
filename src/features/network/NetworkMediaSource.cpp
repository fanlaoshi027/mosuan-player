#include "NetworkMediaSource.h"

#include <QUrl>

NetworkMediaSource::Protocol NetworkMediaSource::protocolFor(const QString& source)
{
    if (source.startsWith(QStringLiteral("smb://"), Qt::CaseInsensitive))
        return Protocol::SMB;

    if (source.startsWith(QStringLiteral("file://"), Qt::CaseInsensitive) ||
        !QUrl(source).isValid() || QUrl(source).scheme().isEmpty())
        return Protocol::LocalFile;

    return Protocol::Unknown;
}

bool NetworkMediaSource::isSMB(const QString& source)
{
    return protocolFor(source) == Protocol::SMB;
}

QString NetworkMediaSource::normalizeSMBUrl(const QString& source)
{
    if (!isSMB(source)) return source;

    QUrl url(source);
    url.setScheme(QStringLiteral("smb"));
    url.setFragment(QString());
    return url.toString(QUrl::FullyEncoded);
}
