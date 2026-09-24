#pragma once

#include <QMainWindow>
#include <memory>

class QTimer;
class VLCInstance;
class VLCPlayer;
class VideoWidget;
class PlayerToolbar;

class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void openVideo();
    void togglePlayPause();
    void updatePlaybackUi();
    void seekVideo(qint64 milliseconds);
    void setPlaybackRate(float rate);
    void setVolume(int volume);
    void toggleFullscreen();

private:
    void setupShortcuts();

    std::unique_ptr<VLCInstance> m_vlc;
    std::unique_ptr<VLCPlayer> m_player;
    std::unique_ptr<QTimer> m_uiTimer;
    VideoWidget* m_video = nullptr;
    PlayerToolbar* m_toolbar = nullptr;
};
