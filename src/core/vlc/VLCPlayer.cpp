#include "VLCPlayer.h"
#include "VLCInstance.h"

#include <QByteArray>
#include <QString>
#include <QtMath>

VLCPlayer::VLCPlayer(VLCInstance* instance, QObject* parent)
    : QObject(parent), m_instance(instance)
{
    if (m_instance && m_instance->instance()) {
        m_player = libvlc_media_player_new(m_instance->instance());
    }
}

VLCPlayer::~VLCPlayer()
{
    if (m_player) libvlc_media_player_release(m_player);
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
        // Video dimensions may only become available after playback starts.
        // Re-apply here so a future persisted crop is not lost.
        if (m_cropEnabled) applyCropGeometry();
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

    if (m_cropEnabled) applyCropGeometry();
}

void VLCPlayer::setCropRect(const QRectF& normalizedRect)
{
    if (!m_player) return;

    const QRectF r = normalizedRect.normalized().intersected(QRectF(0.0, 0.0, 1.0, 1.0));
    if (r.width() <= 0.0 || r.height() <= 0.0) return;

    m_cropRect = r;
    m_cropEnabled = !r.contains(QRectF(0.0, 0.0, 1.0, 1.0)) ||
                    r != QRectF(0.0, 0.0, 1.0, 1.0);
    applyCropGeometry();
}

bool VLCPlayer::applyCropGeometry()
{
    if (!m_player || !m_cropEnabled) return false;

    const int width = videoWidth();
    const int height = videoHeight();
    if (width <= 0 || height <= 0) return false;

    const QRectF r = m_cropRect.normalized().intersected(QRectF(0.0, 0.0, 1.0, 1.0));
    const int x = qBound(0, qRound(r.left() * width), width - 1);
    const int y = qBound(0, qRound(r.top() * height), height - 1);
    const int cropWidth = qBound(1, qRound(r.width() * width), width - x);
    const int cropHeight = qBound(1, qRound(r.height() * height), height - y);

    const QByteArray geometry = QStringLiteral("%1x%2+%3+%4")
        .arg(cropWidth).arg(cropHeight).arg(x).arg(y).toUtf8();
    return libvlc_video_set_crop_geometry(m_player, geometry.constData()) == 0;
}

void VLCPlayer::resetCrop()
{
    m_cropRect = QRectF(0.0, 0.0, 1.0, 1.0);
    m_cropEnabled = false;
    if (m_player) libvlc_video_set_crop_geometry(m_player, nullptr);
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

int VLCPlayer::videoWidth() const
{
    unsigned width = 0;
    unsigned height = 0;
    if (m_player) libvlc_video_get_size(m_player, 0, &width, &height);
    return static_cast<int>(width);
}

int VLCPlayer::videoHeight() const
{
    unsigned width = 0;
    unsigned height = 0;
    if (m_player) libvlc_video_get_size(m_player, 0, &width, &height);
    return static_cast<int>(height);
}

bool VLCPlayer::isPlaying() const
{
    return m_player && libvlc_media_player_is_playing(m_player) != 0;
}
