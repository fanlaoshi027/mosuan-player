#include "SmartInvertController.h"

#include <QtGlobal>

SmartInvertController::SmartInvertController(QObject* parent)
    : QObject(parent)
{
}

void SmartInvertController::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    emit changed();
}

void SmartInvertController::setProtectedRect(const QRectF& rect)
{
    const QRectF normalized = rect.normalized().intersected(QRectF(0.0, 0.0, 1.0, 1.0));
    if (m_protectedRect == normalized) return;
    m_protectedRect = normalized;
    emit changed();
}

void SmartInvertController::setProtectedBrightness(int brightness)
{
    const int value = qBound(0, brightness, 100);
    if (m_protectedBrightness == value) return;
    m_protectedBrightness = value;
    emit changed();
}
