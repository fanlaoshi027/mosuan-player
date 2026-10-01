#include "VLCPlayer.h"
#include "VLCInstance.h"

#include <QByteArray>
#include <QFileInfo>
#include <QMetaObject>
#include <QMutexLocker>
#include <QUrl>
#include <QString>
#include <QtMath>
#include <cstring>

namespace {

libvlc_media_t* createLocalMedia(VLCInstance* instance, const QString& path)
{
    if (!instance || !instance->instance() || path.isEmpty()) return nullptr;

    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) return nullptr;

    const QUrl url = QUrl::fromLocalFile(info.absoluteFilePath());
    const QByteArray encoded = url.toEncoded();
    if (encoded.isEmpty()) return nullptr;

    return libvlc_media_new_location(instance->instance(), encoded.constData());
}

}

VLCPlayer::VLCPlayer(VLCInstance* instance, QObject* parent)
    : QObject(parent), m_instance(instance)
{
    if (m_instance && m_instance->instance()) {
        m_player = libvlc_media_player_new(m_instance->instance());
        if (m_player) {
            m_eventManager = libvlc_media_player_event_manager(m_player);
            if (m_eventManager) {
                libvlc_event_attach(m_eventManager, libvlc_MediaPlayerOpening,
                                    &VLCPlayer::vlcEventCallback, this);
                libvlc_event_attach(m_eventManager, libvlc_MediaPlayerBuffering,
                                    &VLCPlayer::vlcEventCallback, this);
                libvlc_event_attach(m_eventManager, libvlc_MediaPlayerPlaying,
                                    &VLCPlayer::vlcEventCallback, this);
                libvlc_event_attach(m_eventManager, libvlc_MediaPlayerPaused,
                                    &VLCPlayer::vlcEventCallback, this);
                libvlc_event_attach(m_eventManager, libvlc_MediaPlayerEncounteredError,
                                    &VLCPlayer::vlcEventCallback, this);
                libvlc_event_attach(m_eventManager, libvlc_MediaPlayerEndReached,
                                    &VLCPlayer::vlcEventCallback, this);
            }
        }
    }
}

VLCPlayer::~VLCPlayer()
{
    if (m_eventManager) {
        libvlc_event_detach(m_eventManager, libvlc_MediaPlayerOpening,
                            &VLCPlayer::vlcEventCallback, this);
        libvlc_event_detach(m_eventManager, libvlc_MediaPlayerBuffering,
                            &VLCPlayer::vlcEventCallback, this);
        libvlc_event_detach(m_eventManager, libvlc_MediaPlayerPlaying,
                            &VLCPlayer::vlcEventCallback, this);
        libvlc_event_detach(m_eventManager, libvlc_MediaPlayerPaused,
                            &VLCPlayer::vlcEventCallback, this);
        libvlc_event_detach(m_eventManager, libvlc_MediaPlayerEncounteredError,
                            &VLCPlayer::vlcEventCallback, this);
        libvlc_event_detach(m_eventManager, libvlc_MediaPlayerEndReached,
                            &VLCPlayer::vlcEventCallback, this);
    }
    if (m_player) libvlc_media_player_release(m_player);
}

void VLCPlayer::setMediaState(VLCMediaState state)
{
    if (m_mediaState == state) return;
    m_mediaState = state;
    emit mediaStateChanged(m_mediaState);
    emit stateChanged();
}

void VLCPlayer::vlcEventCallback(const libvlc_event_t* event, void* userdata)
{
    if (!event || !userdata) return;
    auto* self = static_cast<VLCPlayer*>(userdata);

    QMetaObject::invokeMethod(self, [self, type = event->type] {
        switch (type) {
        case libvlc_MediaPlayerOpening:
            self->setMediaState(VLCMediaState::Opening);
            break;
        case libvlc_MediaPlayerBuffering:
            self->setMediaState(VLCMediaState::Buffering);
            break;
        case libvlc_MediaPlayerPlaying:
            self->setMediaState(VLCMediaState::Playing);
            break;
        case libvlc_MediaPlayerPaused:
            self->setMediaState(VLCMediaState::Paused);
            break;
        case libvlc_MediaPlayerEncounteredError:
            self->setMediaState(VLCMediaState::Error);
            qWarning() << "VLC media player encountered an error";
            emit self->mediaError(QStringLiteral("VLC 无法解码或播放该视频"));
            break;
        case libvlc_MediaPlayerEndReached:
            self->setMediaState(VLCMediaState::Ended);
            emit self->playbackEnded();
            break;
        default:
            break;
        }
    }, Qt::QueuedConnection);
}

bool VLCPlayer::open(const QString& path)
{
    if (!m_player || path.isEmpty()) return false;

    libvlc_media_t* media = createLocalMedia(m_instance, path);
    if (!media) {
        setMediaState(VLCMediaState::Error);
        emit mediaError(QStringLiteral("无法打开视频文件：文件不存在或路径无效"));
        qWarning() << "Unable to create VLC media for:" << path;
        return false;
    }

    libvlc_media_player_set_media(m_player, media);
    libvlc_media_release(media);
    resetCrop();
    setMediaState(VLCMediaState::Idle);
    return true;
}

void VLCPlayer::play()
{
    if (m_player) {
        libvlc_media_player_play(m_player);
        if (m_cropEnabled && !m_frameProcessingEnabled) applyCropGeometry();
    }
}

void VLCPlayer::pause()
{
    if (m_player) libvlc_media_player_set_pause(m_player, 1);
}

void VLCPlayer::stop()
{
    if (m_player) {
        libvlc_media_player_stop(m_player);
        resetCrop();
        setMediaState(VLCMediaState::Idle);
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
    m_videoOutputWindow = windowId;
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
    if (!m_player || m_frameProcessingEnabled == enabled) return;

    // The old implementation stopped the media player and started it again.
    // That made toggling inversion visibly jump backwards and caused a long
    // decoder/rendering stall. Preserve the exact playback time and rate.
    const qint64 savedTime = libvlc_media_player_get_time(m_player);
    const float savedRate = libvlc_media_player_get_rate(m_player);
    const bool wasPlaying = libvlc_media_player_is_playing(m_player) != 0;

    if (wasPlaying) libvlc_media_player_set_pause(m_player, 1);

    m_frameProcessingEnabled = enabled;
    configureFrameCallbacks(enabled);

    if (enabled) {
        // Frame callbacks must be configured before playback resumes.
        if (m_videoOutputWindow == 0) {
            // No native video target is required when callbacks are active.
        }
    } else if (m_videoOutputWindow) {
        setVideoOutput(m_videoOutputWindow);
    }

    if (savedTime >= 0) {
        libvlc_media_player_set_time(m_player, savedTime);
    }
    if (savedRate > 0.0f) {
        libvlc_media_player_set_rate(m_player, savedRate);
    }
    if (wasPlaying) {
        libvlc_media_player_play(m_player);
    }

    emit stateChanged();
}

void VLCPlayer::configureFrameCallbacks(bool enabled)
{
    if (!m_player) return;
    if (enabled) {
        libvlc_video_set_callbacks(m_player,
                                   &VLCPlayer::frameLock,
                                   &VLCPlayer::frameUnlock,
                                   &VLCPlayer::frameDisplay,
                                   this);
        libvlc_video_set_format_callbacks(m_player, &VLCPlayer::frameFormat,
                                           &VLCPlayer::frameCleanup);
    } else {
        libvlc_video_set_callbacks(m_player, nullptr, nullptr, nullptr, nullptr);
        libvlc_video_set_format_callbacks(m_player, nullptr, nullptr);
        QMutexLocker locker(&m_frameMutex);
        m_frameBuffer.clear();
        m_pendingFrame = QImage();
        m_frameDispatchPending = false;
        m_frameWidth = m_frameHeight = m_framePitch = 0;
    }
}

unsigned VLCPlayer::frameFormat(void** userdata, char* chroma,
                                unsigned* width, unsigned* height,
                                unsigned* pitches, unsigned* lines)
{
    if (!userdata || !*userdata || !chroma || !width || !height || !pitches || !lines) return 0;
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

void VLCPlayer::frameCleanup(void* userdata)
{
    Q_UNUSED(userdata);
}

void* VLCPlayer::frameLock(void* userdata, void** planes)
{
    if (!userdata || !planes) return nullptr;
    auto* self = static_cast<VLCPlayer*>(userdata);
    QMutexLocker locker(&self->m_frameMutex);
    if (self->m_frameBuffer.isEmpty()) {
        *planes = nullptr;
        return nullptr;
    }
    *planes = self->m_frameBuffer.data();
    return self->m_frameBuffer.data();
}

void VLCPlayer::frameUnlock(void* userdata, void* /*picture*/, void* const* /*planes*/)
{
    Q_UNUSED(userdata);
}

void VLCPlayer::frameDisplay(void* userdata, void* /*picture*/)
{
    if (!userdata) return;
    auto* self = static_cast<VLCPlayer*>(userdata);
    const unsigned width = self->m_frameWidth;
    const unsigned height = self->m_frameHeight;
    const unsigned pitch = self->m_framePitch;

    if (width == 0 || height == 0 || pitch == 0) return;

    QImage copy;
    {
        QMutexLocker locker(&self->m_frameMutex);
        if (self->m_frameBuffer.isEmpty()) return;
        QImage frame(reinterpret_cast<const uchar*>(self->m_frameBuffer.constData()),
                     static_cast<int>(width), static_cast<int>(height),
                     static_cast<int>(pitch), QImage::Format_ARGB32);
        copy = frame.copy();
    }

    bool schedule = false;
    {
        QMutexLocker locker(&self->m_frameMutex);
        self->m_pendingFrame = std::move(copy);
        if (!self->m_frameDispatchPending) {
            self->m_frameDispatchPending = true;
            schedule = true;
        }
    }

    if (schedule) {
        QMetaObject::invokeMethod(self, [self] {
            QImage frameToDisplay;
            {
                QMutexLocker locker(&self->m_frameMutex);
                frameToDisplay = std::move(self->m_pendingFrame);
                self->m_frameDispatchPending = false;
            }
            if (!frameToDisplay.isNull()) emit self->frameReady(frameToDisplay);
        }, Qt::QueuedConnection);
    }
}

void VLCPlayer::setCropRect(const QRectF& normalizedRect)
{
    if (!m_player) return;
    const QRectF r = normalizedRect.normalized().intersected(QRectF(0.0, 0.0, 1.0, 1.0));
    if (r.width() <= 0.0 || r.height() <= 0.0) return;
    m_cropRect = r;
    m_cropEnabled = r != QRectF(0.0, 0.0, 1.0, 1.0);
    if (!m_frameProcessingEnabled) applyCropGeometry();
}

bool VLCPlayer::applyCropGeometry()
{
    if (!m_player || !m_cropEnabled || m_frameProcessingEnabled) return false;
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
    libvlc_video_set_crop_geometry(m_player, geometry.constData());
    return true;
}

void VLCPlayer::resetCrop()
{
    m_cropRect = QRectF(0.0, 0.0, 1.0, 1.0);
    m_cropEnabled = false;
    if (m_player && !m_frameProcessingEnabled) libvlc_video_set_crop_geometry(m_player, nullptr);
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
