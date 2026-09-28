#pragma once

#include <QRectF>
#include <QString>

struct VideoProfile
{
    QString mediaPath;
    QRectF cropRect{0.0, 0.0, 1.0, 1.0};
    bool cropEnabled = false;
    bool smartInvert = false;
    QRectF protectedRect{0.0, 0.0, 0.0, 0.0};
    int protectedBrightness = 100;
};
