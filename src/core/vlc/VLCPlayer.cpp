#include "VLCPlayer.h"
#include "VLCInstance.h"

#include <QString>

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
    resetCrop();
    emit stateChanged();
    return true;
}

void VLCPlayer::play()
{
    if (m_player) {
        libvlc_media_player_play(m_player);
        emit stateChanged();
    }
}

void VLCPlayer::pause()
{
    if (m_player) {
        libvlc_media_player_set_pause(m_player, 1);
        emit stateChanged();
    }
}

void VLCPlayer::stop()
{
    if (m_player) {
        libvlc_media_player_stop(m_player);
        resetCrop();
        emit stateChanged();
    }
}

void VLCPlayer::seek(qint64 milliseconds)
{
    if (m_player) libvlc_media_player_set_time(m_player, milliseconds);
}

void VLCPlayer::setRate(float rate)
{
    if (m_player && rate > 0.0f) libvlc_media_player_set_rate(m_player, rate);
}

void VLCPlayer::setVolume(int volume)
{
    if (m_player) libvlc_audio_set_volume(m_player, qBound(0, volume, 100));
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

void VLCPlayer::setCropRect(const QRectF& normalizedRect)
{
    if (!m_player) return;

    const int width = static_cast<int>(libvlc_video_get_width(m_player));
    const int height = static_cast<int>(libvlc_video_get_height(m_player));
    if (width <= 0 || height <= 0) return;

    const QRectF r = normalizedRect.normalized().intersected(QRectF(0.0, 0.0, 1.0, 1.0));
    const int x = qBound(0, qRound(r.left() * width), width - 1);
    const int y = qBound(0, qRound(r.top() * height), height - 1);
    const int cropWidth = qBound(1, qRound(r.width() * width), width - x);
    const int cropHeight = qBound(1, qRound(r.height() * height), height - y);

    const QString geometry = QStringLiteral("%1x%2+%3+%4")
        .arg(cropWidth)
        .arg(cropHeight)
        .arg(x)
        .arg(y);
    const QByteArray utf8 = geometry.toUtf8();
    libvlc_video_set_crop_geometry(m_player, utf8.constData());
}

void VLCPlayer::resetCrop()
{
    if (m_player) {
        libvlc_video_set_crop_geometry(m_player, nullptr);
    }
}

qint64 VLCPlayer::time() const
{
    return m_player ? libvlc_media_player_get_time(m_player) : 0;
}

qint64 VLCPlayer::duration() const
{
    return m_player ? libvlc_media_player_get_length(m_player) : 0;
}

float VLCPlayer::rate() const
{
    return m_player ? libvlc_media_player_get_rate(m_player) : 1.0f;
}

int VLCPlayer::volume() const
{
    return m_player ? libvlc_audio_get_volume(m_player) : 100;
}

bool VLCPlayer::isPlaying() const
{
    return m_player && libvlc_media_player_is_playing(m_player) != 0;
}
