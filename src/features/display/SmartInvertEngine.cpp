#include "SmartInvertEngine.h"

#include <QColor>
#include <QtGlobal>

QImage SmartInvertEngine::process(const QImage& source,
                                  bool enabled,
                                  const QRectF& protectedRect,
                                  int protectedBrightness)
{
    if (source.isNull()) return source;
    if (!enabled && protectedRect.isEmpty()) return source;

    // Keep the original frame geometry and perform one raster pass. The old
    // saturation threshold inverted only grey pixels, which created visible
    // jagged halos around anti-aliased handwriting and coloured edges.
    QImage image = source.format() == QImage::Format_ARGB32
        ? source.copy()
        : source.convertToFormat(QImage::Format_ARGB32);

    const QRectF normalized = protectedRect.normalized().intersected(QRectF(0, 0, 1, 1));
    const bool hasProtected = !normalized.isEmpty()
        && normalized.width() > 0.0 && normalized.height() > 0.0;
    const int brightness = qBound(0, protectedBrightness, 100);

    const int left = hasProtected ? qBound(0, qRound(normalized.left() * image.width()), image.width()) : 0;
    const int top = hasProtected ? qBound(0, qRound(normalized.top() * image.height()), image.height()) : 0;
    const int right = hasProtected ? qBound(0, qRound(normalized.right() * image.width()), image.width()) : 0;
    const int bottom = hasProtected ? qBound(0, qRound(normalized.bottom() * image.height()), image.height()) : 0;

    // Website-style inversion: every pixel outside the protected rectangle is
    // inverted uniformly. This avoids the old saturation cutoff, so edge
    // anti-aliasing remains continuous instead of producing jagged outlines.
    for (int y = 0; y < image.height(); ++y) {
        auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
        const bool rowProtected = hasProtected && y >= top && y < bottom;

        for (int x = 0; x < image.width(); ++x) {
            const bool protectedPixel = rowProtected && x >= left && x < right;
            const QRgb p = row[x];

            if (protectedPixel) {
                if (brightness < 100) {
                    const int r = qRed(p) * brightness / 100;
                    const int g = qGreen(p) * brightness / 100;
                    const int b = qBlue(p) * brightness / 100;
                    row[x] = qRgba(r, g, b, qAlpha(p));
                }
                continue;
            }

            if (enabled) {
                row[x] = qRgba(255 - qRed(p),
                               255 - qGreen(p),
                               255 - qBlue(p),
                               qAlpha(p));
            }
        }
    }

    return image;
}
