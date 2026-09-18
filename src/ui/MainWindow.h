#pragma once

#include <QMainWindow>
#include <memory>

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

private:
    std::unique_ptr<VLCInstance> m_vlc;
    std::unique_ptr<VLCPlayer> m_player;
    VideoWidget* m_video = nullptr;
    PlayerToolbar* m_toolbar = nullptr;
};
