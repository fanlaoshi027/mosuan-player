#include "MediaPathResolver.h"

#include <QFileInfo>
#include <QUrl>

QString MediaPathResolver::toVlcLocation(const QString& filePath)
{
    QFileInfo info(filePath);
    if (!info.exists()) {
        return QString();
    }

    return QUrl::fromLocalFile(info.absoluteFilePath()).toString();
}
