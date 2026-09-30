#pragma once

#include <QImage>
#include "ProtectedRegionManager.h"

namespace Mosuan {

class FrameProcessor
{
public:
    FrameProcessor();

    void setProtectedRegionManager(ProtectedRegionManager* manager);

    QImage process(const QImage& frame, bool smartInvert);

private:
    ProtectedRegionManager* m_regionManager;

    QRgb processPixel(QRgb pixel, bool invert);
    QRgb applyBrightness(QRgb pixel, float brightness);
};

}
