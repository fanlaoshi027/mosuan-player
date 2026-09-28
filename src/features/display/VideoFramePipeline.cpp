#include "VideoFramePipeline.h"
#include "SmartInvertEngine.h"

QImage VideoFramePipeline::process(const QImage& frame) const
{
    return SmartInvertEngine::process(
        frame,
        m_enabled,
        m_protectedRect,
        m_protectedBrightness);
}
