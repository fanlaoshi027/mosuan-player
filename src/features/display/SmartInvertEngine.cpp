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
        QRgb* row = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            const bool protectedPixel = hasProtected && x >= left && x < right && y >= top && y < bottom;
            QRgb& pixel = row[x];
            const int a = qAlpha(pixel);
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            if (enabled && !protectedPixel) {
                // Luma-aware inversion: preserve chroma relationships better than a raw 255-RGB inversion.
                const int luma = (299 * r + 587 * g + 114 * b) / 1000;
                const int invLuma = 255 - luma;
                const double ratio = luma > 4 ? static_cast<double>(invLuma) / luma : 1.0;
                r = qBound(0, qRound(r * ratio), 255);
                g = qBound(0, qRound(g * ratio), 255);
                b = qBound(0, qRound(b * ratio), 255);
            } else if (protectedPixel && brightness < 100) {
                r = r * brightness / 100;
                g = g * brightness / 100;
                b = b * brightness / 100;
            }
            pixel = qRgba(r, g, b, a);
        }
    }
    return image;
}
