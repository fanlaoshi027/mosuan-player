#include "PlayerToolbar.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <climits>

PlayerToolbar::PlayerToolbar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("PlayerToolbar");
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 10, 14, 10);
    layout->setSpacing(7);

    m_playPause = new QPushButton(tr("▶"), this);
    m_playPause->setToolTip(tr("播放 / 暂停 (空格)"));
    m_playPause->setMinimumWidth(42);
    auto* open = new QPushButton(tr("打开"), this);
    open->setToolTip(tr("打开视频 (Ctrl+O)"));

    m_progress = new QSlider(Qt::Horizontal, this);
    m_progress->setRange(0, 0);
    m_progress->setMinimumWidth(280);
    m_time = new QLabel(tr("00:00 / 00:00"), this);
    m_time->setMinimumWidth(100);

    m_rate = new QComboBox(this);
    m_rate->addItems({tr("0.75×"), tr("1.0×"), tr("1.25×"), tr("1.5×"), tr("1.75×"), tr("2.0×")});
    m_rate->setCurrentIndex(1);

    m_volume = new QSlider(Qt::Horizontal, this);
    m_volume->setRange(0, 100);
    m_volume->setValue(100);
    m_volume->setFixedWidth(72);
    m_volume->setToolTip(tr("音量"));

    m_crop = new QPushButton(tr("裁切"), this);
    m_crop->setCheckable(true);
    m_crop->setToolTip(tr("进入裁切模式"));

    auto* smart = new QPushButton(tr("智能反色"), this);
    smart->setCheckable(true);
    smart->setToolTip(tr("智能反色播放"));

    auto* fullscreen = new QPushButton(tr("全屏"), this);
    fullscreen->setToolTip(tr("全屏 (F)"));

    layout->addWidget(m_playPause);
    layout->addWidget(open);
    layout->addWidget(m_progress, 1);
    layout->addWidget(m_time);
    layout->addWidget(m_rate);
    layout->addWidget(m_volume);
    layout->addWidget(m_crop);
    layout->addWidget(smart);
    layout->addWidget(fullscreen);

    connect(open, &QPushButton::clicked, this, &PlayerToolbar::openRequested);
    connect(m_playPause, &QPushButton::clicked, this, &PlayerToolbar::playPauseRequested);
    connect(m_progress, &QSlider::sliderPressed, this, [this] { m_userSeeking = true; });
    connect(m_progress, &QSlider::sliderReleased, this, [this] {
        m_userSeeking = false;
        emit seekRequested(m_progress->value());
    });
    connect(m_progress, &QSlider::sliderMoved, this, [this](int value) {
        if (m_duration > 0)
            m_time->setText(formatTime(value) + tr(" / ") + formatTime(m_duration));
    });
    connect(m_rate, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        QString value = text;
        value.remove(QChar(0x00D7));
        emit rateChanged(value.toFloat());
    });
    connect(m_volume, &QSlider::valueChanged, this, &PlayerToolbar::volumeChanged);
    connect(fullscreen, &QPushButton::clicked, this, &PlayerToolbar::fullscreenRequested);
    connect(m_crop, &QPushButton::clicked, this, &PlayerToolbar::cropRequested);
    connect(smart, &QPushButton::clicked, this, &PlayerToolbar::smartInvertRequested);

    setStyleSheet(R"(
        #PlayerToolbar { background:#141922; border-top:1px solid #29313d; }
        QPushButton, QComboBox { color:#edf2f7; background:#222a35; border:1px solid #303a48;
            border-radius:7px; padding:7px 11px; min-height:28px; }
        QPushButton:hover, QComboBox:hover { background:#2c3644; }
        QPushButton:checked { background:#2f638f; border-color:#477fae; }
        QLabel { color:#cbd3dd; }
        QSlider::groove:horizontal { height:4px; background:#394350; border-radius:2px; }
        QSlider::handle:horizontal { width:12px; margin:-4px 0; border-radius:6px; background:#eef3f7; }
        QComboBox QAbstractItemView { background:#202733; color:#edf2f7; selection-background-color:#2f638f; }
    )");
}

void PlayerToolbar::setDuration(qint64 milliseconds)
{
    m_duration = qMax<qint64>(0, milliseconds);
    m_progress->setRange(0, static_cast<int>(qMin<qint64>(m_duration, INT_MAX)));
    updateTimeLabel();
}

void PlayerToolbar::setPosition(qint64 milliseconds)
{
    if (!m_userSeeking) {
        m_progress->setValue(static_cast<int>(qBound<qint64>(0, milliseconds, m_duration)));
        updateTimeLabel();
    }
}

void PlayerToolbar::setPlaying(bool playing)
{
    m_playPause->setText(playing ? tr("❚❚") : tr("▶"));
}

void PlayerToolbar::setCropActive(bool active)
{
    m_crop->setChecked(active);
}

void PlayerToolbar::updateTimeLabel()
{
    m_time->setText(formatTime(m_progress->value()) + tr(" / ") + formatTime(m_duration));
}

QString PlayerToolbar::formatTime(qint64 milliseconds)
{
    const qint64 totalSeconds = qMax<qint64>(0, milliseconds) / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;
    return hours > 0
        ? QStringLiteral("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'))
        : QStringLiteral("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));
}
