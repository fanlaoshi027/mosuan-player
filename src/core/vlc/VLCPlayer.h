#pragma once

#include <QObject>
#include <QRectF>
#include <QImage>
#include <QMutex>
#include <QByteArray>
#include <vlc/vlc.h>

class VLCInstance;

class VLCPlayer final : public QObject
{
    Q_OBJECT
public:
    explicit VLCPlayer(VLCInstance* instance, QObject* parent = nullptr);
    ~VLCPlayer() override;

    bool open(const QString& path);
    void play();
    void pause();
    void stop();
    void seek(qint64 milliseconds);
    void setRate(float rate);
    void setVolume(int volume);
    void setVideoOutput(WId windowId);
    void setCropRect(const QRectF& normalizedRect);
    void resetCrop();

    // Opt-in frame path. Native VLC output remains the default for performance.
    void setFrameProcessingEnabled(bool enabled);
    bool frameProcessingEnabled() const { return m_frameProcessingEnabled; }

    qint64 time() const;
    qint64 duration() const;
    float rate() const;
    int volume() const;
    int videoWidth() const;
    int videoHeight() const;
    bool isPlaying() const;

signals:
    void stateChanged();
    void playbackEnded();
    void frameReady(const QImage& frame);

private:
    static void vlcEventCallback(const libvlc_event_t* event, void* userdata);
    static void* frameLock(void* userdata, void** planes);
    static void frameUnlock(void* userdata, void* picture, void* const* planes);
    static void frameDisplay(void* userdata, void* picture);
    static unsigned frameFormat(void** userdata, char* chroma,
                                unsigned* width, unsigned* height,
                                unsigned* pitches, unsigned* lines);
    void configureFrameCallbacks(bool enabled);
    bool applyCropGeometry();

    VLCInstance* m_instance = nullptr;
    libvlc_media_player_t* m_player = nullptr;
    libvlc_event_manager_t* m_eventManager = nullptr;
    QRectF m_cropRect{0.0, 0.0, 1.0, 1.0};
    bool m_cropEnabled = false;

    bool m_frameProcessingEnabled = false;
    QMutex m_frameMutex;
    QByteArray m_frameBuffer;
    QImage m_pendingFrame;
    bool m_frameDispatchPending = false;
    unsigned m_frameWidth = 0;
    unsigned m_frameHeight = 0;
    unsigned m_framePitch = 0;
};
