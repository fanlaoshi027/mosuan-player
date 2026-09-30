#pragma once

#include <QWidget>

class QPushButton;
class QSlider;
class QLabel;
class QComboBox;

class PlayerToolbar final : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerToolbar(QWidget* parent = nullptr);
    void setDuration(qint64 milliseconds);
    void setPosition(qint64 milliseconds);
    void setPlaying(bool playing);
    void setCropActive(bool active);
    void setProtectedActive(bool active);

signals:
    void openRequested();
    void playPauseRequested();
    void seekRequested(qint64 milliseconds);
    void rateChanged(float rate);
    void volumeChanged(int volume);
    void fullscreenRequested();
    void cropRequested();
    void smartInvertRequested();
    void protectedRequested();

private:
    void updateTimeLabel();
    static QString formatTime(qint64 milliseconds);
    QPushButton* m_playPause = nullptr;
    QPushButton* m_crop = nullptr;
    QPushButton* m_protected = nullptr;
    QSlider* m_progress = nullptr;
    QSlider* m_volume = nullptr;
    QLabel* m_time = nullptr;
    QComboBox* m_rate = nullptr;
    qint64 m_duration = 0;
    bool m_userSeeking = false;
};
