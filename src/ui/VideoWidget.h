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
    enum class CropHandle { None, Move, TopLeft, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left };

    QPointF toNormalized(const QPointF& point) const;
    QPointF toPixels(const QPointF& point) const;
    CropHandle hitTest(const QPointF& pixelPoint) const;
    void updateCursor(CropHandle handle);
    void updateCropFromPointer(const QPointF& normalizedPoint);

    bool m_cropMode = false;
    bool m_draggingCrop = false;
    QPointF m_cropStart;
    QRectF m_cropRect{0.0, 0.0, 1.0, 1.0};
    QRectF m_dragStartRect;
    QPointF m_dragStartPoint;
    CropHandle m_activeHandle = CropHandle::None;
};
