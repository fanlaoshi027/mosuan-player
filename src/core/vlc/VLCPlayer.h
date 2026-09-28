#pragma once

#include <QObject>
#include <QRectF>
#include <QString>
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

private:
    static void vlcEventCallback(const libvlc_event_t* event, void* userdata);
    bool applyCropGeometry();

    VLCInstance* m_instance = nullptr;
    libvlc_media_player_t* m_player = nullptr;
    libvlc_event_manager_t* m_eventManager = nullptr;
    QRectF m_cropRect{0.0, 0.0, 1.0, 1.0};
    bool m_cropEnabled = false;
};
