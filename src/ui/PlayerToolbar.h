#pragma once

#include <QWidget>
#include "PlayerTool.h"

class QPushButton;

class PlayerToolbar final : public QWidget
{
    Q_OBJECT
public:
    explicit PlayerToolbar(QWidget* parent = nullptr);

signals:
    void openRequested();
    void playPauseRequested();
    void toolChanged(PlayerTool tool);

private:
    void addToolButton(QHBoxLayout* layout, const QString& text, PlayerTool tool);

    QPushButton* m_playPause = nullptr;
};
