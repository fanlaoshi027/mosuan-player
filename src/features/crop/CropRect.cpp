#include "CropRect.h"

QRectF CropRectMath::clamp(const QRectF& input)
{
    QRectF rect = input.normalized();
    rect.setLeft(qBound(0.0, rect.left(), 1.0));
    rect.setTop(qBound(0.0, rect.top(), 1.0));
    rect.setRight(qBound(0.0, rect.right(), 1.0));
    rect.setBottom(qBound(0.0, rect.bottom(), 1.0));
    return rect.normalized();
}

QRectF CropRectMath::fromDrag(const QPointF& start, const QPointF& current)
{
    return clamp(QRectF(start, current));
}
