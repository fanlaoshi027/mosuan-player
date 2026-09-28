#pragma once

#include <QImage>
#include <QRectF>

class SmartInvertEngine final
{
public:
    static QImage process(const QImage& source,
                          bool enabled,
                          const QRectF& protectedRect = QRectF(),
                          int protectedBrightness = 100);
};
