#include "VideoProfileStore.h"

VideoProfile& VideoProfileStore::profileFor(const QString& mediaPath)
{
    auto it = m_profiles.find(mediaPath);
    if (it == m_profiles.end()) {
        VideoProfile profile;
        profile.mediaPath = mediaPath;
        it = m_profiles.insert(mediaPath, profile);
    }
    return it.value();
}

const VideoProfile* VideoProfileStore::find(const QString& mediaPath) const
{
    const auto it = m_profiles.constFind(mediaPath);
    return it == m_profiles.cend() ? nullptr : &it.value();
}

void VideoProfileStore::clear(const QString& mediaPath)
{
    m_profiles.remove(mediaPath);
}
