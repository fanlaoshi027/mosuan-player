#pragma once

#include "VideoProfile.h"
#include <QHash>
#include <QString>

class VideoProfileStore final
{
public:
    VideoProfile& profileFor(const QString& mediaPath);
    const VideoProfile* find(const QString& mediaPath) const;
    void clear(const QString& mediaPath);

private:
    QHash<QString, VideoProfile> m_profiles;
};
