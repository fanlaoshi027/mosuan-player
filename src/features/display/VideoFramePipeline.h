#pragma once

#include <QImage>
#include <QRectF>

class VideoFramePipeline final
{
public:
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    void setProtectedRect(const QRectF& rect) { m_protectedRect = rect; }
    void setProtectedBrightness(int value) { m_protectedBrightness = value; }

    QImage process(const QImage& frame) const;

private:
    bool m_enabled = false;
    QRectF m_protectedRect;
    int m_protectedBrightness = 100;
};
