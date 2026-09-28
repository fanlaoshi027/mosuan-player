#pragma once

#include <QObject>
#include <QByteArray>
#include <QImage>
#include <QMutex>
#include <QRectF>
#include <QSize>
#include <vlc/vlc.h>

class LibVLCFrameBridge final : public QObject
{
    Q_OBJECT
public:
    explicit LibVLCFrameBridge(QObject* parent = nullptr);
    ~LibVLCFrameBridge() override;

    bool attach(libvlc_media_player_t* player);
    void detach();
    void setProcessingEnabled(bool enabled);
    void setProtectedRect(const QRectF& rect);
    void setProtectedBrightness(int brightness);
    QImage takeLatestFrame();
    QSize frameSize() const;

signals:
    void frameReady();

private:
    static void* lockCallback(void* opaque, void** planes);
    static void unlockCallback(void* opaque, void* picture, void* const* planes);
    static void displayCallback(void* opaque, void* picture);
    static unsigned formatCallback(void** opaque, char* chroma, unsigned* width,
                                   unsigned* height, unsigned* pitches, unsigned* lines);
    static void cleanupCallback(void* opaque);

    libvlc_media_player_t* m_player = nullptr;
    bool m_attached = false;
    bool m_enabled = false;
    QRectF m_protectedRect;
    int m_protectedBrightness = 100;
    mutable QMutex m_mutex;
    QImage m_latestFrame;
    QSize m_size;
    QByteArray m_buffer;
};
