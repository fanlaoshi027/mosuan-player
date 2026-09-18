#include "MainWindow.h"

#include "../core/vlc/VLCInstance.h"
#include "../core/vlc/VLCPlayer.h"
#include "PlayerToolbar.h"
#include "VideoWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_vlc(std::make_unique<VLCInstance>())
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
}

MainWindow::~MainWindow() = default;

void MainWindow::openVideo()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("打开视频"),
        QString(),
        tr("视频文件 (*.mp4 *.mkv *.mov *.avi *.webm);;所有文件 (*)"));

    if (path.isEmpty() || !m_player) return;

    if (m_player->open(path)) {
        m_player->play();
    } else {
        QMessageBox::warning(this, tr("打开失败"), tr("无法打开该视频。"));
    }
}

void MainWindow::togglePlayPause()
{
    if (!m_player) return;
    if (m_player->isPlaying()) {
        m_player->pause();
    } else {
        m_player->play();
    }
}
