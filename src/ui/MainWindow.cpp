#include "MainWindow.h"

#include "../core/vlc/VLCInstance.h"
#include "../core/vlc/VLCPlayer.h"
#include "../features/playlist/PlaylistModel.h"
#include "../features/profile/VideoProfileStore.h"
#include "PlayerToolbar.h"
#include "VideoWidget.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_vlc(std::make_unique<VLCInstance>()),
      m_uiTimer(std::make_unique<QTimer>(this)),
      m_playlist(std::make_unique<PlaylistModel>()),
      m_profiles(std::make_unique<VideoProfileStore>())
{
    setWindowTitle(tr("Mosuan Player"));
    resize(1200, 760);

    auto* central = new QWidget(this);
    setupPlaylistUi(central);
    setCentralWidget(central);

    if (!m_vlc->initialize()) {
        QMessageBox::critical(this, tr("Mosuan Player"), tr("LibVLC 初始化失败，请检查 LibVLC 安装。"));
        return;
    }

    m_player = std::make_unique<VLCPlayer>(m_vlc.get(), this);
    m_player->setVideoOutput(m_video->nativeVideoId());

    connect(m_toolbar, &PlayerToolbar::openRequested, this, &MainWindow::openVideo);
    connect(m_toolbar, &PlayerToolbar::playPauseRequested, this, &MainWindow::togglePlayPause);
    connect(m_toolbar, &PlayerToolbar::seekRequested, this, &MainWindow::seekVideo);
    connect(m_toolbar, &PlayerToolbar::rateChanged, this, &MainWindow::setPlaybackRate);
    connect(m_toolbar, &PlayerToolbar::volumeChanged, this, &MainWindow::setVolume);
    connect(m_toolbar, &PlayerToolbar::fullscreenRequested, this, &MainWindow::toggleFullscreen);
    connect(m_toolbar, &PlayerToolbar::cropRequested, this, [this] {
        if (!m_video || !m_player) return;
        const bool enable = !m_video->cropMode();
        m_video->setCropMode(enable);
        m_toolbar->setCropActive(enable);
        if (!enable) m_player->resetCrop();
    });
    connect(m_toolbar, &PlayerToolbar::smartInvertRequested, this, &MainWindow::toggleSmartInvert);
    connect(m_video, &VideoWidget::cropChanged, this, [this](const QRectF& rect) {
        if (!m_player || m_currentPath.isEmpty()) return;
        m_player->setCropRect(rect);
        auto& profile = m_profiles->profileFor(m_currentPath);
        profile.cropRect = rect;
        profile.cropEnabled = rect.width() < 0.999 || rect.height() < 0.999;
    });

    m_uiTimer->setInterval(250);
    connect(m_uiTimer.get(), &QTimer::timeout, this, &MainWindow::updatePlaybackUi);
    m_uiTimer->start();
    setupShortcuts();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupPlaylistUi(QWidget* central)
{
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* playerPanel = new QWidget(central);
    auto* playerLayout = new QVBoxLayout(playerPanel);
    playerLayout->setContentsMargins(0, 0, 0, 0);
    playerLayout->setSpacing(0);
    m_video = new VideoWidget(playerPanel);
    m_toolbar = new PlayerToolbar(playerPanel);
    playerLayout->addWidget(m_video, 1);
    playerLayout->addWidget(m_toolbar);

    auto* side = new QWidget(central);
    side->setObjectName("PlaylistPanel");
    side->setMinimumWidth(260);
    side->setMaximumWidth(340);
    auto* sideLayout = new QVBoxLayout(side);
    sideLayout->setContentsMargins(10, 10, 10, 10);
    sideLayout->setSpacing(8);

    auto* titleRow = new QHBoxLayout;
    auto* title = new QLabel(tr("播放列表"), side);
    auto* addButton = new QPushButton(tr("＋ 添加"), side);
    titleRow->addWidget(title);
    titleRow->addStretch();
    titleRow->addWidget(addButton);
    sideLayout->addLayout(titleRow);

    m_playlistWidget = new QListWidget(side);
    m_playlistWidget->setObjectName("PlaylistWidget");
    sideLayout->addWidget(m_playlistWidget, 1);
    root->addWidget(playerPanel, 1);
    root->addWidget(side);

    connect(addButton, &QPushButton::clicked, this, &MainWindow::openVideos);
    connect(m_playlistWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::playPlaylistItem);

    central->setStyleSheet(R"(
        #PlaylistPanel { background:#171c24; border-left:1px solid #29313d; }
        #PlaylistPanel QLabel { color:#eef2f6; font-size:15px; font-weight:600; }
        #PlaylistPanel QPushButton { color:#eef2f6; background:#26303c; border:1px solid #364252; border-radius:7px; padding:6px 10px; }
        #PlaylistPanel QPushButton:hover { background:#314052; }
        #PlaylistWidget { background:#12171e; border:1px solid #29313d; border-radius:7px; color:#dce3ea; outline:0; padding:4px; }
        #PlaylistWidget::item { padding:9px 8px; border-radius:5px; }
        #PlaylistWidget::item:selected { background:#2f638f; color:white; }
        #PlaylistWidget::item:hover { background:#252e39; }
    )");
}

void MainWindow::setupShortcuts()
{
    auto* space = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(space, &QShortcut::activated, this, &MainWindow::togglePlayPause);
    auto* fullscreen = new QShortcut(QKeySequence(Qt::Key_F), this);
    connect(fullscreen, &QShortcut::activated, this, &MainWindow::toggleFullscreen);
    auto* escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escape, &QShortcut::activated, this, [this] { if (isFullScreen()) showNormal(); });
    auto* open = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this);
    connect(open, &QShortcut::activated, this, &MainWindow::openVideo);
}

void MainWindow::openVideo()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("打开视频"), QString(),
        tr("视频文件 (*.mp4 *.mkv *.mov *.avi *.webm);;所有文件 (*)"));
    if (path.isEmpty()) return;
    m_playlist->add(path);
    m_playlistWidget->clear();
    for (const QString& item : m_playlist->items()) m_playlistWidget->addItem(QFileInfo(item).fileName());
    loadVideoPath(path);
}

void MainWindow::openVideos()
{
    const QStringList paths = QFileDialog::getOpenFileNames(this, tr("添加视频"), QString(),
        tr("视频文件 (*.mp4 *.mkv *.mov *.avi *.webm);;所有文件 (*)"));
    if (paths.isEmpty()) return;
    for (const QString& path : paths) m_playlist->add(path);
    m_playlistWidget->clear();
    for (const QString& item : m_playlist->items()) m_playlistWidget->addItem(QFileInfo(item).fileName());
    if (m_currentPath.isEmpty()) loadVideoPath(paths.first());
}

void MainWindow::playPlaylistItem(QListWidgetItem* item)
{
    if (!item) return;
    const int index = m_playlistWidget->row(item);
    if (index >= 0 && index < m_playlist->size()) loadVideoPath(m_playlist->at(index));
}

void MainWindow::loadVideoPath(const QString& path)
{
    if (!m_player || path.isEmpty()) return;
    m_currentPath = path;
    if (!m_player->open(path)) {
        QMessageBox::warning(this, tr("打开失败"), tr("无法打开该视频。"));
        return;
    }
    m_player->setVideoOutput(m_video->nativeVideoId());
    m_player->play();
    applyVideoProfile(path);
    const int index = m_playlist->indexOf(path);
    if (index >= 0) m_playlistWidget->setCurrentRow(index);
}

void MainWindow::applyVideoProfile(const QString& path)
{
    const auto* profile = m_profiles->find(path);
    m_video->setCropMode(false);
    m_toolbar->setCropActive(false);
    m_player->resetCrop();
    setSmartInvertEnabled(profile && profile->smartInvert);
    if (profile && profile->cropEnabled) m_player->setCropRect(profile->cropRect);
}

void MainWindow::togglePlayPause()
{
    if (!m_player) return;
    if (m_player->isPlaying()) m_player->pause(); else m_player->play();
    updatePlaybackUi();
}

void MainWindow::updatePlaybackUi()
{
    if (!m_player || !m_toolbar) return;
    m_toolbar->setDuration(m_player->duration());
    m_toolbar->setPosition(m_player->time());
    m_toolbar->setPlaying(m_player->isPlaying());
}

void MainWindow::seekVideo(qint64 milliseconds) { if (m_player) m_player->seek(milliseconds); }
void MainWindow::setPlaybackRate(float rate) { if (m_player) m_player->setRate(rate); }
void MainWindow::setVolume(int volume) { if (m_player) m_player->setVolume(volume); }
void MainWindow::toggleFullscreen() { if (isFullScreen()) showNormal(); else showFullScreen(); }
void MainWindow::toggleSmartInvert() { setSmartInvertEnabled(!m_smartInvert); }

void MainWindow::setSmartInvertEnabled(bool enabled)
{
    m_smartInvert = enabled;
    if (!m_currentPath.isEmpty()) m_profiles->profileFor(m_currentPath).smartInvert = enabled;
    if (m_video) { m_video->setProperty("smartInvert", enabled); m_video->update(); }
}

void MainWindow::applyVideoDisplayGeometry() { if (m_video) m_video->update(); }
