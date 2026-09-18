#include "PlayerToolbar.h"

#include <QHBoxLayout>
#include <QPushButton>

PlayerToolbar::PlayerToolbar(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(6);

    m_playPause = new QPushButton(tr("播放 / 暂停"), this);
    auto* open = new QPushButton(tr("打开视频"), this);
    auto* crop = new QPushButton(tr("裁切"), this);
    auto* smart = new QPushButton(tr("智能反色"), this);
    auto* fullscreen = new QPushButton(tr("全屏"), this);

    layout->addWidget(m_playPause);
    layout->addWidget(open);
    layout->addSpacing(10);
    layout->addWidget(crop);
    layout->addWidget(smart);
    layout->addWidget(fullscreen);
    layout->addStretch();

    connect(open, &QPushButton::clicked, this, &PlayerToolbar::openRequested);
    connect(m_playPause, &QPushButton::clicked, this, &PlayerToolbar::playPauseRequested);

    setStyleSheet(R"(
        QWidget { background:#171c24; }
        QPushButton {
            color:#f2f5f8;
            background:#242b36;
            border:0;
            border-radius:7px;
            padding:8px 12px;
        }
        QPushButton:hover { background:#303947; }
    )");
}
