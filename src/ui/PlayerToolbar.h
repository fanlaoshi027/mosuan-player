#pragma once

#include <QWidget>

class QPushButton;

class PlayerToolbar final : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerToolbar(QWidget* parent = nullptr);

signals:
    void openRequested();
    void playPauseRequested();

private:
    QPushButton* m_playPause = nullptr;
};
