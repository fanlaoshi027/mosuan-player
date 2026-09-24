#pragma once

#include <QPointF>
#include <QRectF>
#include <QWidget>

class VideoWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget* parent = nullptr);

    WId nativeVideoId() const { return winId(); }

    void setCropMode(bool enabled);
    bool cropMode() const { return m_cropMode; }
    QRectF cropRect() const { return m_cropRect; }
    void resetCrop();

signals:
    void cropChanged(const QRectF& normalizedRect);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QPointF toNormalized(const QPointF& point) const;

    bool m_cropMode = false;
    bool m_draggingCrop = false;
    QPointF m_cropStart;
    QRectF m_cropRect{0.0, 0.0, 1.0, 1.0};
};
