#pragma once

#include <QRectF>

// Region excluded from smart inversion.
// Coordinates are normalized (0.0 - 1.0) relative to video frame.
struct ProtectedRegion
{
    bool enabled = false;
    QRectF rect;
    double brightness = 1.0;

    bool contains(double x, double y) const
    {
        return enabled && rect.contains(x, y);
    }
};
