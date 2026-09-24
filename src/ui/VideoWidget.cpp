#include "VideoWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>

VideoWidget::VideoWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(640, 360);
    setMouseTracking(true);
    setStyleSheet("background:#101318;");
}

void VideoWidget::setCropMode(bool enabled)
{
    m_cropMode = enabled;
    m_draggingCrop = false;
    setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

void VideoWidget::resetCrop()
{
    m_cropRect = QRectF(0.0, 0.0, 1.0, 1.0);
    m_draggingCrop = false;
    emit cropChanged(m_cropRect);
    update();
}

QPointF VideoWidget::toNormalized(const QPointF& point) const
{
    if (width() <= 0 || height() <= 0) return {};
    return QPointF(
        qBound(0.0, point.x() / static_cast<double>(width()), 1.0),
        qBound(0.0, point.y() / static_cast<double>(height()), 1.0));
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    if (!m_cropMode) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF pixelRect(
        m_cropRect.left() * width(),
        m_cropRect.top() * height(),
        m_cropRect.width() * width(),
        m_cropRect.height() * height());

    painter.fillRect(rect(), QColor(0, 0, 0, 105));
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(pixelRect, Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::white);
    painter.drawRect(pixelRect);

    constexpr double handle = 10.0;
    for (const QPointF& p : {pixelRect.topLeft(), pixelRect.topRight(),
                             pixelRect.bottomLeft(), pixelRect.bottomRight()}) {
        painter.drawRect(QRectF(p.x() - handle / 2.0, p.y() - handle / 2.0,
                                handle, handle));
    }
}

void VideoWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_cropMode || event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    m_draggingCrop = true;
    m_cropStart = toNormalized(event->position());
    m_cropRect = QRectF(m_cropStart, m_cropStart);
    update();
}

void VideoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_cropMode || !m_draggingCrop) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QPointF current = toNormalized(event->position());
    m_cropRect = QRectF(m_cropStart, current).normalized();
    m_cropRect.setLeft(qBound(0.0, m_cropRect.left(), 1.0));
    m_cropRect.setTop(qBound(0.0, m_cropRect.top(), 1.0));
    m_cropRect.setRight(qBound(0.0, m_cropRect.right(), 1.0));
    m_cropRect.setBottom(qBound(0.0, m_cropRect.bottom(), 1.0));
    emit cropChanged(m_cropRect);
    update();
}

void VideoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_cropMode || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    m_draggingCrop = false;
    emit cropChanged(m_cropRect);
    update();
}
