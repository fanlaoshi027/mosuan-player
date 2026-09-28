#include "SmartInvertEngine.h"

#include <QtGlobal>

QImage SmartInvertEngine::process(const QImage& source,
                                  bool enabled,
                                  const QRectF& protectedRect,
                                  int protectedBrightness)
{
    if (source.isNull()) return source;
    if (!enabled && protectedRect.isEmpty()) return source;

    // Performance-first path: integer-only per-pixel work. Avoid floating
    // point/luma analysis because this function may run for every video frame.
    QImage image = source.convertToFormat(QImage::Format_ARGB32);
    const QRectF normalized = protectedRect.normalized().intersected(QRectF(0, 0, 1, 1));
    const bool hasProtected = !normalized.isEmpty() && normalized.width() > 0 && normalized.height() > 0;
    const int brightness = qBound(0, protectedBrightness, 100);

    const int left = hasProtected ? qBound(0, qRound(normalized.left() * image.width()), image.width()) : 0;
    const int top = hasProtected ? qBound(0, qRound(normalized.top() * image.height()), image.height()) : 0;
    const int right = hasProtected ? qBound(0, qRound(normalized.right() * image.width()), image.width()) : 0;
    const int bottom = hasProtected ? qBound(0, qRound(normalized.bottom() * image.height()), image.height()) : 0;

    for (int y = 0; y < image.height(); ++y) {
        QRgb* row = reinterpret_cast<QRgb*>(image.scanLine(y));
        const bool rowProtected = hasProtected && y >= top && y < bottom;

        for (int x = 0; x < image.width(); ++x) {
            QRgb p = row[x];
            const bool protectedPixel = rowProtected && x >= left && x < right;

            if (enabled && !protectedPixel) {
                // Fast RGB inversion. No floating point and no per-pixel luma calculation.
                row[x] = qRgba(255 - qRed(p),
                               255 - qGreen(p),
                               255 - qBlue(p),
                               qAlpha(p));
            } else if (protectedPixel && brightness < 100) {
                row[x] = qRgba(qRed(p) * brightness / 100,
                               qGreen(p) * brightness / 100,
                               qBlue(p) * brightness / 100,
                               qAlpha(p));
            }
        }
    }

    return image;
}
