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

private:
    VLCInstance* m_instance = nullptr;
    libvlc_media_player_t* m_player = nullptr;
};
