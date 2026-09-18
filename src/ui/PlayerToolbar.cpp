#include "PlayerToolbar.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>

PlayerToolbar::PlayerToolbar(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(8);

    m_playPause = new QPushButton(tr("播放"), this);
    auto* open = new QPushButton(tr("打开视频"), this);
    m_progress = new QSlider(Qt::Horizontal, this);
    m_progress->setRange(0, 0);
    m_progress->setMinimumWidth(260);
    m_time = new QLabel(tr("00:00 / 00:00"), this);
    m_rate = new QComboBox(this);
    m_rate->addItems({tr("0.75×"), tr("1.0×"), tr("1.25×"), tr("1.5×"), tr("1.75×"), tr("2.0×")});
    m_rate->setCurrentIndex(1);

    m_volume = new QSlider(Qt::Horizontal, this);
    m_volume->setRange(0, 100);
    m_volume->setValue(100);
    m_volume->setFixedWidth(80);

    auto* crop = new QPushButton(tr("裁切"), this);
    auto* smart = new QPushButton(tr("智能反色"), this);
    auto* fullscreen = new QPushButton(tr("全屏"), this);

    layout->addWidget(m_playPause);
    layout->addWidget(open);
    layout->addWidget(m_progress, 1);
    layout->addWidget(m_time);
    layout->addWidget(m_rate);
    layout->addWidget(m_volume);
    layout->addWidget(crop);
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
        if (m_duration > 0) {
            m_time->setText(formatTime(value) + tr(" / ") + formatTime(m_duration));
        }
    });
    connect(m_rate, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        QString value = text;
        value.remove(QChar(0x00D7));
        emit rateChanged(value.toFloat());
    });
    connect(m_volume, &QSlider::valueChanged, this, &PlayerToolbar::volumeChanged);
    connect(fullscreen, &QPushButton::clicked, this, &PlayerToolbar::fullscreenRequested);
    connect(crop, &QPushButton::clicked, this, &PlayerToolbar::cropRequested);
    connect(smart, &QPushButton::clicked, this, &PlayerToolbar::smartInvertRequested);

    setStyleSheet(R"(
        QWidget { background:#171c24; }
        QPushButton, QComboBox {
            color:#f2f5f8;
            background:#242b36;
            border:0;
            border-radius:7px;
            padding:8px 12px;
        }
        QPushButton:hover, QComboBox:hover { background:#303947; }
        QLabel { color:#d9dee5; }
        QSlider::groove:horizontal { height:4px; background:#3a424e; border-radius:2px; }
        QSlider::handle:horizontal { width:12px; margin:-4px 0; border-radius:6px; background:#f2f5f8; }
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
    m_playPause->setText(playing ? tr("暂停") : tr("播放"));
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
