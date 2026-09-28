#include "VLCPlayer.h"
#include "VLCInstance.h"

#include <QByteArray>
#include <QMetaObject>
#include <QMutexLocker>
#include <QString>
#include <QtMath>
#include <cstring>

VLCPlayer::VLCPlayer(VLCInstance* instance, QObject* parent)
    : QObject(parent), m_instance(instance)
{
    if (m_instance && m_instance->instance()) {
        m_player = libvlc_media_player_new(m_instance->instance());
        if (m_player) {
            m_eventManager = libvlc_media_player_event_manager(m_player);
            if (m_eventManager) {
                libvlc_event_attach(m_eventManager, libvlc_MediaPlayerEndReached,
                                    &VLCPlayer::vlcEventCallback, this);
            }
        }
    }
}

VLCPlayer::~VLCPlayer()
{
    if (m_eventManager) {
        libvlc_event_detach(m_eventManager, libvlc_MediaPlayerEndReached,
                            &VLCPlayer::vlcEventCallback, this);
    }
    if (m_player) libvlc_media_player_release(m_player);
}

void VLCPlayer::vlcEventCallback(const libvlc_event_t* event, void* userdata)
{
    if (!event || !userdata) return;
    if (event->type != libvlc_MediaPlayerEndReached) return;
    auto* self = static_cast<VLCPlayer*>(userdata);
    QMetaObject::invokeMethod(self, [self] {
        emit self->playbackEnded();
        emit self->stateChanged();
    }, Qt::QueuedConnection);
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
    if (!m_player || m_frameProcessingEnabled) return;
#ifdef _WIN32
    libvlc_media_player_set_hwnd(m_player, reinterpret_cast<void*>(windowId));
#elif defined(__APPLE__)
    libvlc_media_player_set_nsobject(m_player, reinterpret_cast<void*>(windowId));
#else
    libvlc_media_player_set_xwindow(m_player, static_cast<uint32_t>(windowId));
#endif
    if (m_cropEnabled) applyCropGeometry();
}

void VLCPlayer::setFrameProcessingEnabled(bool enabled)
{
    if (m_frameProcessingEnabled == enabled) return;
    const bool wasPlaying = isPlaying();
    if (wasPlaying && m_player) libvlc_media_player_stop(m_player);

    m_frameProcessingEnabled = enabled;
    configureFrameCallbacks(enabled);

    if (wasPlaying && m_player) libvlc_media_player_play(m_player);
    emit stateChanged();
}

void VLCPlayer::configureFrameCallbacks(bool enabled)
{
    if (!m_player) return;
    if (enabled) {
        // RV32 gives us one 32-bit pixel per source pixel and avoids YUV conversion
        // in our own processing path. This path is opt-in; native VLC output stays default.
        libvlc_video_set_callbacks(m_player,
                                   &VLCPlayer::frameLock,
                                   &VLCPlayer::frameUnlock,
                                   &VLCPlayer::frameDisplay,
                                   this);
        libvlc_video_set_format_callbacks(m_player, &VLCPlayer::frameFormat, this);
    } else {
        libvlc_video_set_callbacks(m_player, nullptr, nullptr, nullptr, nullptr);
        libvlc_video_set_format_callbacks(m_player, nullptr, nullptr);
        QMutexLocker locker(&m_frameMutex);
        m_frameBuffer.clear();
        m_frameWidth = m_frameHeight = m_framePitch = 0;
    }
}

unsigned VLCPlayer::frameFormat(void** userdata, char* chroma,
                                unsigned* width, unsigned* height,
                                unsigned* pitches, unsigned* lines)
{
    if (!userdata || !*userdata || !chroma || !width || !height || !pitches || !lines)
        return 0;

    auto* self = static_cast<VLCPlayer*>(*userdata);
    std::memcpy(chroma, "RV32", 4);

    self->m_frameWidth = *width;
    self->m_frameHeight = *height;
    self->m_framePitch = (*width) * 4;
    *pitches = self->m_framePitch;
    *lines = *height;

    QMutexLocker locker(&self->m_frameMutex);
    self->m_frameBuffer.resize(static_cast<qsizetype>(self->m_framePitch) * self->m_frameHeight);
    return 1;
}

void* VLCPlayer::frameLock(void* userdata, void** planes)
{
    if (!userdata || !planes) return nullptr;
    auto* self = static_cast<VLCPlayer*>(userdata);
    self->m_frameMutex.lock();
    if (self->m_frameBuffer.isEmpty()) {
        self->m_frameMutex.unlock();
        *planes = nullptr;
        return nullptr;
    }
    *planes = self->m_frameBuffer.data();
    return self->m_frameBuffer.data();
}

void VLCPlayer::frameUnlock(void* userdata, void* /*picture*/, void* const* /*planes*/)
{
    if (!userdata) return;
    auto* self = static_cast<VLCPlayer*>(userdata);
    // Keep the buffer locked until display() has copied the frame.
    Q_UNUSED(self);
}

void VLCPlayer::frameDisplay(void* userdata, void* /*picture*/)
{
    if (!userdata) return;
    auto* self = static_cast<VLCPlayer*>(userdata);
    const unsigned width = self->m_frameWidth;
    const unsigned height = self->m_frameHeight;
    const unsigned pitch = self->m_framePitch;

    if (width == 0 || height == 0 || pitch == 0 || self->m_frameBuffer.isEmpty()) {
        self->m_frameMutex.unlock();
        return;
    }

    QImage frame(reinterpret_cast<const uchar*>(self->m_frameBuffer.constData()),
                 static_cast<int>(width), static_cast<int>(height),
                 static_cast<int>(pitch), QImage::Format_ARGB32);
    // The queued signal needs ownership after the callback returns, so detach once here.
    QImage copy = frame.copy();
    self->m_frameMutex.unlock();

    QMetaObject::invokeMethod(self, [self, copy = std::move(copy)]() mutable {
        emit self->frameReady(copy);
    }, Qt::QueuedConnection);
}

void VLCPlayer::setCropRect(const QRectF& normalizedRect)
{
    if (!m_player) return;
    const QRectF r = normalizedRect.normalized().intersected(QRectF(0.0, 0.0, 1.0, 1.0));
    if (r.width() <= 0.0 || r.height() <= 0.0) return;
    m_cropRect = r;
    m_cropEnabled = r != QRectF(0.0, 0.0, 1.0, 1.0);
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

qint64 VLCPlayer::time() const { return m_player ? libvlc_media_player_get_time(m_player) : 0; }
qint64 VLCPlayer::duration() const { return m_player ? libvlc_media_player_get_length(m_player) : 0; }
float VLCPlayer::rate() const { return m_player ? libvlc_media_player_get_rate(m_player) : 1.0f; }
int VLCPlayer::volume() const { return m_player ? libvlc_audio_get_volume(m_player) : 100; }

int VLCPlayer::videoWidth() const
{
    unsigned width = 0, height = 0;
    if (m_player) libvlc_video_get_size(m_player, 0, &width, &height);
    return static_cast<int>(width);
}

int VLCPlayer::videoHeight() const
{
    unsigned width = 0, height = 0;
    if (m_player) libvlc_video_get_size(m_player, 0, &width, &height);
    return static_cast<int>(height);
}

bool VLCPlayer::isPlaying() const
{
    return m_player && libvlc_media_player_is_playing(m_player) != 0;
}
