#include "VideoWidget.h"

#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

namespace {
constexpr double kHandle = 12.0;
constexpr double kHit = 18.0;
constexpr double kMinSize = 0.03;
}

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
    m_activeHandle = CropHandle::None;
    setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

void VideoWidget::resetCrop()
{
    m_cropRect = QRectF(0.0, 0.0, 1.0, 1.0);
    m_draggingCrop = false;
    m_activeHandle = CropHandle::None;
    emit cropChanged(m_cropRect);
    emit cropCommitted(m_cropRect);
    update();
}

QPointF VideoWidget::toNormalized(const QPointF& point) const
{
    if (width() <= 0 || height() <= 0) return {};
    return QPointF(qBound(0.0, point.x() / static_cast<double>(width()), 1.0),
                   qBound(0.0, point.y() / static_cast<double>(height()), 1.0));
}

QPointF VideoWidget::toPixels(const QPointF& point) const
{
    return QPointF(point.x() * width(), point.y() * height());
}

VideoWidget::CropHandle VideoWidget::hitTest(const QPointF& p) const
{
    const QRectF r(toPixels(m_cropRect.topLeft()), toPixels(m_cropRect.bottomRight()));
    const QPointF tl = r.topLeft();
    const QPointF t(r.center().x(), r.top());
    const QPointF tr = r.topRight();
    const QPointF rr(r.right(), r.center().y());
    const QPointF br = r.bottomRight();
    const QPointF b(r.center().x(), r.bottom());
    const QPointF bl = r.bottomLeft();
    const QPointF l(r.left(), r.center().y());

    if (QLineF(p, tl).length() <= kHit) return CropHandle::TopLeft;
    if (QLineF(p, tr).length() <= kHit) return CropHandle::TopRight;
    if (QLineF(p, br).length() <= kHit) return CropHandle::BottomRight;
    if (QLineF(p, bl).length() <= kHit) return CropHandle::BottomLeft;
    if (QLineF(p, t).length() <= kHit && p.x() >= r.left() && p.x() <= r.right()) return CropHandle::Top;
    if (QLineF(p, rr).length() <= kHit && p.y() >= r.top() && p.y() <= r.bottom()) return CropHandle::Right;
    if (QLineF(p, b).length() <= kHit && p.x() >= r.left() && p.x() <= r.right()) return CropHandle::Bottom;
    if (QLineF(p, l).length() <= kHit && p.y() >= r.top() && p.y() <= r.bottom()) return CropHandle::Left;
    if (r.contains(p)) return CropHandle::Move;
    return CropHandle::None;
}

void VideoWidget::updateCursor(CropHandle handle)
{
    switch (handle) {
    case CropHandle::Move: setCursor(Qt::SizeAllCursor); break;
    case CropHandle::Top:
    case CropHandle::Bottom: setCursor(Qt::SizeVerCursor); break;
    case CropHandle::Left:
    case CropHandle::Right: setCursor(Qt::SizeHorCursor); break;
    case CropHandle::TopLeft:
    case CropHandle::BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case CropHandle::TopRight:
    case CropHandle::BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    default: setCursor(Qt::CrossCursor); break;
    }
}

void VideoWidget::updateCropFromPointer(const QPointF& p)
{
    QRectF r = m_dragStartRect;
    const double dx = p.x() - m_dragStartPoint.x();
    const double dy = p.y() - m_dragStartPoint.y();

    switch (m_activeHandle) {
    case CropHandle::Move:
        r.moveLeft(qBound(0.0, r.left() + dx, 1.0 - r.width()));
        r.moveTop(qBound(0.0, r.top() + dy, 1.0 - r.height()));
        break;
    case CropHandle::TopLeft:
        r.setLeft(qBound(0.0, r.left() + dx, r.right() - kMinSize));
        r.setTop(qBound(0.0, r.top() + dy, r.bottom() - kMinSize));
        break;
    case CropHandle::Top:
        r.setTop(qBound(0.0, r.top() + dy, r.bottom() - kMinSize));
        break;
    case CropHandle::TopRight:
        r.setRight(qBound(r.left() + kMinSize, r.right() + dx, 1.0));
        r.setTop(qBound(0.0, r.top() + dy, r.bottom() - kMinSize));
        break;
    case CropHandle::Right:
        r.setRight(qBound(r.left() + kMinSize, r.right() + dx, 1.0));
        break;
    case CropHandle::BottomRight:
        r.setRight(qBound(r.left() + kMinSize, r.right() + dx, 1.0));
        r.setBottom(qBound(r.top() + kMinSize, r.bottom() + dy, 1.0));
        break;
    case CropHandle::Bottom:
        r.setBottom(qBound(r.top() + kMinSize, r.bottom() + dy, 1.0));
        break;
    case CropHandle::BottomLeft:
        r.setLeft(qBound(0.0, r.left() + dx, r.right() - kMinSize));
        r.setBottom(qBound(r.top() + kMinSize, r.bottom() + dy, 1.0));
        break;
    case CropHandle::Left:
        r.setLeft(qBound(0.0, r.left() + dx, r.right() - kMinSize));
        break;
    case CropHandle::None:
        return;
    }

    m_cropRect = r.normalized();
    emit cropChanged(m_cropRect);
    update();
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    if (!m_cropMode) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QRectF pixelRect(m_cropRect.left() * width(), m_cropRect.top() * height(),
                           m_cropRect.width() * width(), m_cropRect.height() * height());

    painter.fillRect(rect(), QColor(0, 0, 0, 105));
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(pixelRect, Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::white);
    painter.drawRect(pixelRect);

    for (const QPointF& p : {pixelRect.topLeft(), pixelRect.topRight(),
                             pixelRect.bottomRight(), pixelRect.bottomLeft()}) {
        painter.drawRect(QRectF(p.x() - kHandle / 2.0, p.y() - kHandle / 2.0,
                                kHandle, kHandle));
    }
}

void VideoWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_cropMode || event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const CropHandle hit = hitTest(event->position());
    if (hit == CropHandle::None) {
        m_activeHandle = CropHandle::None;
        m_draggingCrop = true;
        m_cropStart = toNormalized(event->position());
        m_cropRect = QRectF(m_cropStart, m_cropStart);
    } else {
        m_activeHandle = hit;
        m_draggingCrop = true;
        m_dragStartRect = m_cropRect;
        m_dragStartPoint = toNormalized(event->position());
    }
    updateCursor(hit);
    update();
}

void VideoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_cropMode) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const QPointF normalized = toNormalized(event->position());
    if (!m_draggingCrop) {
        updateCursor(hitTest(event->position()));
        return;
    }

    if (m_activeHandle == CropHandle::None) {
        m_cropRect = QRectF(m_cropStart, normalized).normalized();
        m_cropRect.setLeft(qBound(0.0, m_cropRect.left(), 1.0));
        m_cropRect.setTop(qBound(0.0, m_cropRect.top(), 1.0));
        m_cropRect.setRight(qBound(0.0, m_cropRect.right(), 1.0));
        m_cropRect.setBottom(qBound(0.0, m_cropRect.bottom(), 1.0));
        emit cropChanged(m_cropRect);
        update();
        return;
    }

    updateCropFromPointer(normalized);
}

void VideoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_cropMode || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    m_draggingCrop = false;
    emit cropChanged(m_cropRect);
    emit cropCommitted(m_cropRect);
    updateCursor(hitTest(event->position()));
    update();
}
