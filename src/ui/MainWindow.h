#pragma once

#include <QMainWindow>
#include <QRectF>
#include <memory>

class QListWidget;
class QPushButton;
class QEvent;
class QListWidgetItem;
class QTimer;
class VLCInstance;
class VLCPlayer;
class VideoWidget;
class PlayerToolbar;
class PlaylistModel;
class VideoProfileStore;
class VideoFramePipeline;

class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void openVideo();
    void openVideos();
    void playPlaylistItem(QListWidgetItem* item);
    void togglePlayPause();
    void updatePlaybackUi();
    void seekVideo(qint64 milliseconds);
    void setPlaybackRate(float rate);
    void setVolume(int volume);
    void toggleFullscreen();
    void toggleSmartInvert();
    void playNextItem();

private:
    void setupShortcuts();
    void applyStudentPolicy();
    bool eventFilter(QObject* watched, QEvent* event) override;
    void setupPlaylistUi(QWidget* central);
    void refreshPlaylistWidget();
    void loadVideoPath(const QString& path);
    void applyVideoProfile(const QString& path);
    void applyVideoDisplayGeometry();
    void setSmartInvertEnabled(bool enabled);

    std::unique_ptr<VLCInstance> m_vlc;
    std::unique_ptr<VLCPlayer> m_player;
    std::unique_ptr<QTimer> m_uiTimer;
    std::unique_ptr<PlaylistModel> m_playlist;
    std::unique_ptr<VideoProfileStore> m_profiles;
    std::unique_ptr<VideoFramePipeline> m_framePipeline;
    VideoWidget* m_video = nullptr;
    PlayerToolbar* m_toolbar = nullptr;
    QListWidget* m_playlistWidget = nullptr;
    bool m_smartInvert = false;
    bool m_studentMode = false;
    bool m_disableContextMenu = false;
    bool m_disableUnsafeShortcuts = false;
    bool m_denyArbitraryFiles = false;
    bool m_allowPlaybackShortcuts = true;
    QPushButton* m_addButton = nullptr;
    QString m_currentPath;
};
