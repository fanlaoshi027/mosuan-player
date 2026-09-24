#pragma once

#include <QPointF>
#include <QRectF>

struct CropRect
{
    // Normalized video coordinates: 0..1.
    QRectF normalized = QRectF(0.0, 0.0, 1.0, 1.0);

    bool isFullFrame() const
    {
        return normalized == QRectF(0.0, 0.0, 1.0, 1.0);
    }
};

class CropRectMath final
{
public:
    static QRectF clamp(const QRectF& rect);
    static QRectF fromDrag(const QPointF& start, const QPointF& current);
};
