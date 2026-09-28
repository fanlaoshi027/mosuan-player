#include "SmartInvertEngine.h"

#include <QtGlobal>

QImage SmartInvertEngine::process(const QImage& source,
                                  bool enabled,
                                  const QRectF& protectedRect,
                                  int protectedBrightness)
{
    if (source.isNull()) return source;
    if (!enabled && protectedRect.isEmpty()) return source;

    QImage image = source.convertToFormat(QImage::Format_ARGB32);
    const QRectF normalized = protectedRect.normalized().intersected(QRectF(0, 0, 1, 1));
    const bool hasProtected = !normalized.isEmpty() && normalized.width() > 0 && normalized.height() > 0;
    const int brightness = qBound(0, protectedBrightness, 100);

    const int left = hasProtected ? qBound(0, qRound(normalized.left() * image.width()), image.width()) : 0;
    const int top = hasProtected ? qBound(0, qRound(normalized.top() * image.height()), image.height()) : 0;
    const int right = hasProtected ? qBound(0, qRound(normalized.right() * image.width()), image.width()) : 0;
    const int bottom = hasProtected ? qBound(0, qRound(normalized.bottom() * image.height()), image.height()) : 0;

    for (int y = 0; y < image.height(); ++y) {
        auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
        const bool rowProtected = hasProtected && y >= top && y < bottom;

        for (int x = 0; x < image.width(); ++x) {
            const bool protectedPixel = rowProtected && x >= left && x < right;
            QRgb p = row[x];

            if (enabled && !protectedPixel) {
                // QRgb is 0xAARRGGBB. XOR changes RGB in one integer operation
                // while preserving alpha. This is substantially cheaper than
                // three channel extraction/packing operations.
                row[x] = p ^ 0x00FFFFFFu;
            } else if (protectedPixel && brightness < 100) {
                const int r = qRed(p) * brightness / 100;
                const int g = qGreen(p) * brightness / 100;
                const int b = qBlue(p) * brightness / 100;
                row[x] = qRgba(r, g, b, qAlpha(p));
            }
        }
    }

    return image;
}
