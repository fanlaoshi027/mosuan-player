#include "LibVLCFrameBridge.h"
#include "SmartInvertEngine.h"

#include <QMutexLocker>
#include <QtGlobal>
#include <cstring>

LibVLCFrameBridge::LibVLCFrameBridge(QObject* parent)
    : QObject(parent)
{
}

LibVLCFrameBridge::~LibVLCFrameBridge()
{
    detach();
}

bool LibVLCFrameBridge::attach(libvlc_media_player_t* player)
{
    if (!player) return false;
    detach();
    m_player = player;
    libvlc_media_player_retain(m_player);

    libvlc_video_set_callbacks(m_player, &lockCallback, &unlockCallback,
                               &displayCallback, this);
    libvlc_video_set_format_callbacks(m_player, &formatCallback, &cleanupCallback);
    m_attached = true;
    return true;
}

void LibVLCFrameBridge::detach()
{
    if (!m_player) return;
    libvlc_video_set_callbacks(m_player, nullptr, nullptr, nullptr, nullptr);
    libvlc_video_set_format_callbacks(m_player, nullptr, nullptr);
    libvlc_media_player_release(m_player);
    m_player = nullptr;
    m_attached = false;
    QMutexLocker lock(&m_mutex);
    m_latestFrame = QImage();
    m_buffer.clear();
}

void LibVLCFrameBridge::setProcessingEnabled(bool enabled) { m_enabled = enabled; }
void LibVLCFrameBridge::setProtectedRect(const QRectF& rect) { m_protectedRect = rect.normalized().intersected(QRectF(0, 0, 1, 1)); }
void LibVLCFrameBridge::setProtectedBrightness(int brightness) { m_protectedBrightness = qBound(0, brightness, 100); }

QImage LibVLCFrameBridge::takeLatestFrame()
{
    QMutexLocker lock(&m_mutex);
    QImage frame = m_latestFrame;
    m_latestFrame = QImage();
    return frame;
}

QSize LibVLCFrameBridge::frameSize() const
{
    QMutexLocker lock(&m_mutex);
    return m_size;
}

void* LibVLCFrameBridge::lockCallback(void* opaque, void** planes)
{
    auto* self = static_cast<LibVLCFrameBridge*>(opaque);
    if (!self || !planes || self->m_buffer.isEmpty()) return nullptr;
    *planes = self->m_buffer.data();
    return self->m_buffer.data();
}

void LibVLCFrameBridge::unlockCallback(void*, void*, void* const*)
{
}

void LibVLCFrameBridge::displayCallback(void* opaque, void*)
{
    auto* self = static_cast<LibVLCFrameBridge*>(opaque);
    if (!self) return;
    QImage frame;
    {
        QMutexLocker lock(&self->m_mutex);
        if (self->m_size.isEmpty() || self->m_buffer.isEmpty()) return;
        frame = QImage(reinterpret_cast<const uchar*>(self->m_buffer.constData()),
                       self->m_size.width(), self->m_size.height(),
                       self->m_size.width() * 4, QImage::Format_ARGB32).copy();
    }
    if (self->m_enabled) {
        frame = SmartInvertEngine::process(frame, true, self->m_protectedRect,
                                           self->m_protectedBrightness);
    }
    {
        QMutexLocker lock(&self->m_mutex);
        self->m_latestFrame = frame;
    }
    emit self->frameReady();
}

unsigned LibVLCFrameBridge::formatCallback(void** opaque, char* chroma, unsigned* width,
                                            unsigned* height, unsigned* pitches, unsigned* lines)
{
    if (!opaque || !*opaque || !chroma || !width || !height || !pitches || !lines) return 0;
    auto* self = static_cast<LibVLCFrameBridge*>(*opaque);
    if (*width == 0 || *height == 0) return 0;

    std::memcpy(chroma, "RV32", 4);
    *pitches = *width * 4;
    *lines = *height;
    {
        QMutexLocker lock(&self->m_mutex);
        self->m_size = QSize(static_cast<int>(*width), static_cast<int>(*height));
        self->m_buffer.resize(static_cast<int>(*pitches) * static_cast<int>(*lines));
    }
    return 1;
}

void LibVLCFrameBridge::cleanupCallback(void* opaque)
{
    auto* self = static_cast<LibVLCFrameBridge*>(opaque);
    if (!self) return;
    QMutexLocker lock(&self->m_mutex);
    self->m_buffer.clear();
    self->m_size = QSize();
}
