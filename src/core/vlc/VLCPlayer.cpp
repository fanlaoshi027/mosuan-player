#include "VLCPlayer.h"
#include "VLCInstance.h"

VLCPlayer::VLCPlayer(VLCInstance* instance, QObject* parent)
    : QObject(parent), m_instance(instance)
{
    if (m_instance && m_instance->instance()) {
        m_player = libvlc_media_player_new(m_instance->instance());
    }
}

VLCPlayer::~VLCPlayer()
{
    if (m_player) {
        libvlc_media_player_release(m_player);
    }
}

bool VLCPlayer::open(const QString& path)
{
    if (!m_player || path.isEmpty()) return false;

    libvlc_media_t* media = libvlc_media_new_path(
        m_instance->instance(), path.toUtf8().constData());
    if (!media) return false;

    libvlc_media_player_set_media(m_player, media);
    libvlc_media_release(media);
    return true;
}

void VLCPlayer::play()
{
    if (m_player) libvlc_media_player_play(m_player);
}

void VLCPlayer::pause()
{
    if (m_player) libvlc_media_player_set_pause(m_player, 1);
}

void VLCPlayer::stop()
{
    if (m_player) libvlc_media_player_stop(m_player);
}

void VLCPlayer::seek(qint64 milliseconds)
{
    if (m_player) libvlc_media_player_set_time(m_player, milliseconds);
}

void VLCPlayer::setRate(float rate)
{
    if (m_player) libvlc_media_player_set_rate(m_player, rate);
}

void VLCPlayer::setVideoOutput(WId windowId)
{
    if (!m_player) return;
#ifdef _WIN32
    libvlc_media_player_set_hwnd(m_player, reinterpret_cast<void*>(windowId));
#elif defined(__APPLE__)
    libvlc_media_player_set_nsobject(m_player, reinterpret_cast<void*>(windowId));
#else
    libvlc_media_player_set_xwindow(m_player, static_cast<uint32_t>(windowId));
#endif
}

bool VLCPlayer::isPlaying() const
{
    return m_player && libvlc_media_player_is_playing(m_player) != 0;
}
