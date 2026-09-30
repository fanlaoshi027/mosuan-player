#include "SmartInvertEngine.h"

#include <QColor>
#include <QtGlobal>

namespace {
constexpr int kNeutralSaturation = 38;
}

QImage SmartInvertEngine::process(const QImage& source,
                                  bool enabled,
                                  const QRectF& protectedRect,
                                  int protectedBrightness)
{
    if (source.isNull()) return source;
    if (!enabled && protectedRect.isEmpty()) return source;

    // VLC supplies RV32/ARGB32 frames. Keep that format so the processing path
    // performs one copy instead of convertToFormat() followed by another copy.
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

    // Smart mode: invert neutral/greyscale pixels only. This is much more
    // suitable for lesson videos than a blind RGB inversion: white paper and
    // black handwriting become dark paper and light handwriting, while faces,
    // coloured pens and other colourful video content retain their colours.
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

            if (!enabled) continue;

            const int maxChannel = qMax(qRed(p), qMax(qGreen(p), qBlue(p)));
            const int minChannel = qMin(qRed(p), qMin(qGreen(p), qBlue(p)));
            const int saturation = maxChannel - minChannel;

            if (saturation <= kNeutralSaturation) {
                row[x] = qRgba(255 - qRed(p),
                               255 - qGreen(p),
                               255 - qBlue(p),
                               qAlpha(p));
            }
        }
    }

    return image;
}
