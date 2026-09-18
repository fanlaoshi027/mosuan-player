#include "PlayerToolbar.h"

#include <QButtonGroup>
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
    layout->addWidget(m_playPause);
    layout->addWidget(open);
    layout->addSpacing(10);

    // Tool order is intentional: selection first, then canvas operations, then drawing.
    addToolButton(layout, tr("选择"), PlayerTool::Select);
    addToolButton(layout, tr("移动画布"), PlayerTool::PanCanvas);
    addToolButton(layout, tr("框选放大"), PlayerTool::ZoomBox);
    addToolButton(layout, tr("笔"), PlayerTool::Pen);
    addToolButton(layout, tr("激光"), PlayerTool::Laser);
    addToolButton(layout, tr("橡皮"), PlayerTool::Eraser);
    addToolButton(layout, tr("直线"), PlayerTool::Line);

    layout->addStretch();

    auto* crop = new QPushButton(tr("裁切"), this);
    auto* smart = new QPushButton(tr("智能"), this);
    auto* fullscreen = new QPushButton(tr("全屏"), this);
    layout->addWidget(crop);
    layout->addWidget(smart);
    layout->addWidget(fullscreen);

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
        QPushButton:checked { background:#e88a2b; color:#ffffff; }
    )");
}

void PlayerToolbar::addToolButton(QHBoxLayout* layout, const QString& text, PlayerTool tool)
{
    auto* button = new QPushButton(text, this);
    button->setCheckable(true);
    button->setAutoExclusive(true);
    button->setProperty("playerTool", static_cast<int>(tool));
    layout->addWidget(button);

    connect(button, &QPushButton::clicked, this, [this, tool]() {
        emit toolChanged(tool);
    });

    // Selection is the initial and highest-priority tool.
    if (tool == PlayerTool::Select) {
        button->setChecked(true);
    }
}
