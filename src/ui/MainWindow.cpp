#include "MainWindow.h"

#include "../core/vlc/VLCInstance.h"
#include "../core/vlc/VLCPlayer.h"
#include "PlayerToolbar.h"
#include "VideoWidget.h"

#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QShortcut>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_vlc(std::make_unique<VLCInstance>()),
      m_uiTimer(std::make_unique<QTimer>(this))
{
    setWindowTitle(tr("Mosuan Player"));
    resize(1100, 700);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_video = new VideoWidget(central);
    m_toolbar = new PlayerToolbar(central);
    layout->addWidget(m_video, 1);
    layout->addWidget(m_toolbar, 0);
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
        if (!enable) {
            m_video->resetCrop();
            m_player->resetCrop();
        }
    });

    connect(m_toolbar, &PlayerToolbar::smartInvertRequested,
            this, &MainWindow::toggleSmartInvert);

    connect(m_video, &VideoWidget::cropChanged, this, [this](const QRectF& rect) {
        if (m_player && m_video && m_video->cropMode()) {
            m_player->setCropRect(rect);
        }
    });

    m_uiTimer->setInterval(250);
    connect(m_uiTimer.get(), &QTimer::timeout, this, &MainWindow::updatePlaybackUi);
    m_uiTimer->start();

    setupShortcuts();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupShortcuts()
{
    auto* space = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(space, &QShortcut::activated, this, &MainWindow::togglePlayPause);

    auto* fullscreen = new QShortcut(QKeySequence(Qt::Key_F), this);
    connect(fullscreen, &QShortcut::activated, this, &MainWindow::toggleFullscreen);

    auto* escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escape, &QShortcut::activated, this, [this] {
        if (isFullScreen()) showNormal();
    });

    auto* open = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this);
    connect(open, &QShortcut::activated, this, &MainWindow::openVideo);
}

void MainWindow::openVideo()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("打开视频"), QString(),
        tr("视频文件 (*.mp4 *.mkv *.mov *.avi *.webm);;所有文件 (*)"));
    if (path.isEmpty() || !m_player) return;

    m_video->setCropMode(false);
    m_video->resetCrop();
    m_toolbar->setCropActive(false);
    setSmartInvertEnabled(false);

    if (m_player->open(path)) {
        m_player->setVideoOutput(m_video->nativeVideoId());
        m_player->play();
    } else {
        QMessageBox::warning(this, tr("打开失败"), tr("无法打开该视频。"));
    }
}

void MainWindow::togglePlayPause()
{
    if (!m_player) return;
    if (m_player->isPlaying()) m_player->pause();
    else m_player->play();
    updatePlaybackUi();
}

void MainWindow::updatePlaybackUi()
{
    if (!m_player || !m_toolbar) return;
    m_toolbar->setDuration(m_player->duration());
    m_toolbar->setPosition(m_player->time());
    m_toolbar->setPlaying(m_player->isPlaying());
}

void MainWindow::seekVideo(qint64 milliseconds)
{
    if (m_player) m_player->seek(milliseconds);
}

void MainWindow::setPlaybackRate(float rate)
{
    if (m_player) m_player->setRate(rate);
}

void MainWindow::setVolume(int volume)
{
    if (m_player) m_player->setVolume(volume);
}

void MainWindow::toggleFullscreen()
{
    if (isFullScreen()) showNormal();
    else showFullScreen();
}

void MainWindow::toggleSmartInvert()
{
    setSmartInvertEnabled(!m_smartInvert);
}

void MainWindow::setSmartInvertEnabled(bool enabled)
{
    m_smartInvert = enabled;
    if (!m_video) return;

    // Keep the effect at the video-widget level so it can later be combined
    // with the exclusion rectangle without changing VLC decoding.
    m_video->setProperty("smartInvert", enabled);
    m_video->update();
}

void MainWindow::applyVideoDisplayGeometry()
{
    // Reserved for aspect-ratio-aware video geometry. VLC owns the native
    // video surface; this hook keeps display geometry separate from crop state.
    if (m_video) m_video->update();
}
