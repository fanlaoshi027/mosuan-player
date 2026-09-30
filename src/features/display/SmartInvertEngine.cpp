#include "SmartInvertEngine.h"

#include <QtGlobal>

QImage SmartInvertEngine::process(const QImage& source,
                                  bool enabled,
                                  const QRectF& protectedRect,
                                  int protectedBrightness)
{
    if (source.isNull()) return source;
    if (!enabled && protectedRect.isEmpty()) return source;

    // VLC already supplies RV32/ARGB32 frames. Keep the same format so the
    // processing path performs one copy instead of convertToFormat() + copy.
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

    // Fast path: no protected region. This is the common full-frame mode.
    if (enabled && !hasProtected) {
        for (int y = 0; y < image.height(); ++y) {
            auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < image.width(); ++x)
                row[x] ^= 0x00FFFFFFu;
        }
        return image;
    }

    for (int y = 0; y < image.height(); ++y) {
        auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
        const bool rowProtected = hasProtected && y >= top && y < bottom;

        for (int x = 0; x < image.width(); ++x) {
            const bool protectedPixel = rowProtected && x >= left && x < right;
            QRgb p = row[x];

            if (enabled && !protectedPixel) {
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
