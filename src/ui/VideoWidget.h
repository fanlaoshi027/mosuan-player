#pragma once

#include <QWidget>
#include "PlayerTool.h"

class VideoWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget* parent = nullptr);

    WId nativeVideoId() const { return winId(); }
    void setTool(PlayerTool tool);
    PlayerTool tool() const { return m_tool; }

signals:
    void zoomChanged(double factor);
    void canvasPanChanged(const QPoint& delta);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    bool isTemporaryPanActive() const;

    PlayerTool m_tool = PlayerTool::Select;
    bool m_middlePanning = false;
    bool m_spacePanning = false;
    QPoint m_lastPanPosition;
    double m_zoomFactor = 1.0;
};
