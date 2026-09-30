#include "FrameProcessor.h"
#include <QtGlobal>

namespace Mosuan {

FrameProcessor::FrameProcessor()
    : m_regionManager(nullptr)
{
}

void FrameProcessor::setProtectedRegionManager(ProtectedRegionManager* manager)
{
    m_regionManager = manager;
}

QImage FrameProcessor::process(const QImage& frame, bool smartInvert)
{
    if (frame.isNull())
        return frame;

    QImage result = frame.convertToFormat(QImage::Format_ARGB32);

    for (int y = 0; y < result.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(result.scanLine(y));

        for (int x = 0; x < result.width(); ++x) {
            bool protectedArea = false;
            float brightness = 1.0f;

            if (m_regionManager) {
                auto info = m_regionManager->regionAt(x, y, result.size());
                protectedArea = info.enabled;
                brightness = info.brightness;
            }

            if (protectedArea) {
                line[x] = applyBrightness(line[x], brightness);
            } else {
                line[x] = processPixel(line[x], smartInvert);
            }
        }
    }

    return result;
}

QRgb FrameProcessor::processPixel(QRgb pixel, bool invert)
{
    if (!invert)
        return pixel;

    int r = qRed(pixel);
    int g = qGreen(pixel);
    int b = qBlue(pixel);

    int maxValue = qMax(r, qMax(g, b));
    int minValue = qMin(r, qMin(g, b));

    // 彩色内容尽量保持
    if (maxValue - minValue > 40)
        return pixel;

    return qRgba(255 - r, 255 - g, 255 - b, qAlpha(pixel));
}

QRgb FrameProcessor::applyBrightness(QRgb pixel, float brightness)
{
    return qRgba(
        qBound(0, int(qRed(pixel) * brightness), 255),
        qBound(0, int(qGreen(pixel) * brightness), 255),
        qBound(0, int(qBlue(pixel) * brightness), 255),
        qAlpha(pixel)
    );
}

}
