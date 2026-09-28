#pragma once

#include "VideoProfile.h"
#include <QHash>
#include <QString>

class VideoProfileStore final
{
public:
    explicit VideoProfileStore(QString storageFile = {});

    VideoProfile& profileFor(const QString& mediaPath);
    const VideoProfile* find(const QString& mediaPath) const;
    void clear(const QString& mediaPath);
    bool load();
    bool save() const;

private:
    QString m_storageFile;
    QHash<QString, VideoProfile> m_profiles;
};
