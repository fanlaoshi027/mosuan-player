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

VideoWidget::VideoWidget(QWidget* parent) : QWidget(parent)
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
    if (enabled) m_protectedMode = false;
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

void VideoWidget::setProtectedMode(bool enabled)
{
    m_protectedMode = enabled;
    m_draggingProtected = false;
    m_activeProtectedHandle = ProtectedHandle::None;
    if (enabled) m_cropMode = false;
    setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

void VideoWidget::setProtectedRect(const QRectF& rect)
{
    m_protectedRect = rect.normalized().intersected(QRectF(0.0, 0.0, 1.0, 1.0));
    update();
}

void VideoWidget::resetProtectedRect()
{
    m_protectedRect = QRectF();
    m_draggingProtected = false;
    m_activeProtectedHandle = ProtectedHandle::None;
    emit protectedRectChanged(m_protectedRect);
    emit protectedRectCommitted(m_protectedRect);
    update();
}

void VideoWidget::setProcessedFrame(const QImage& frame) { m_processedFrame = frame; update(); }
void VideoWidget::clearProcessedFrame() { m_processedFrame = QImage(); update(); }

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
    const QPointF tl=r.topLeft(), t(r.center().x(),r.top()), tr=r.topRight(), rr(r.right(),r.center().y());
    const QPointF br=r.bottomRight(), b(r.center().x(),r.bottom()), bl=r.bottomLeft(), l(r.left(),r.center().y());
    if (QLineF(p,tl).length()<=kHit) return CropHandle::TopLeft;
    if (QLineF(p,tr).length()<=kHit) return CropHandle::TopRight;
    if (QLineF(p,br).length()<=kHit) return CropHandle::BottomRight;
    if (QLineF(p,bl).length()<=kHit) return CropHandle::BottomLeft;
    if (QLineF(p,t).length()<=kHit && p.x()>=r.left() && p.x()<=r.right()) return CropHandle::Top;
    if (QLineF(p,rr).length()<=kHit && p.y()>=r.top() && p.y()<=r.bottom()) return CropHandle::Right;
    if (QLineF(p,b).length()<=kHit && p.x()>=r.left() && p.x()<=r.right()) return CropHandle::Bottom;
    if (QLineF(p,l).length()<=kHit && p.y()>=r.top() && p.y()<=r.bottom()) return CropHandle::Left;
    if (r.contains(p)) return CropHandle::Move;
    return CropHandle::None;
}

VideoWidget::ProtectedHandle VideoWidget::hitTestProtected(const QPointF& p) const
{
    if (m_protectedRect.isNull()) return ProtectedHandle::None;
    const QRectF r(toPixels(m_protectedRect.topLeft()), toPixels(m_protectedRect.bottomRight()));
    const QPointF tl=r.topLeft(), t(r.center().x(),r.top()), tr=r.topRight(), rr(r.right(),r.center().y());
    const QPointF br=r.bottomRight(), b(r.center().x(),r.bottom()), bl=r.bottomLeft(), l(r.left(),r.center().y());
    if (QLineF(p,tl).length()<=kHit) return ProtectedHandle::TopLeft;
    if (QLineF(p,tr).length()<=kHit) return ProtectedHandle::TopRight;
    if (QLineF(p,br).length()<=kHit) return ProtectedHandle::BottomRight;
    if (QLineF(p,bl).length()<=kHit) return ProtectedHandle::BottomLeft;
    if (QLineF(p,t).length()<=kHit && p.x()>=r.left() && p.x()<=r.right()) return ProtectedHandle::Top;
    if (QLineF(p,rr).length()<=kHit && p.y()>=r.top() && p.y()<=r.bottom()) return ProtectedHandle::Right;
    if (QLineF(p,b).length()<=kHit && p.x()>=r.left() && p.x()<=r.right()) return ProtectedHandle::Bottom;
    if (QLineF(p,l).length()<=kHit && p.y()>=r.top() && p.y()<=r.bottom()) return ProtectedHandle::Left;
    if (r.contains(p)) return ProtectedHandle::Move;
    return ProtectedHandle::None;
}

void VideoWidget::updateCursor(CropHandle handle)
{
    switch(handle) {
    case CropHandle::Move: setCursor(Qt::SizeAllCursor); break;
    case CropHandle::Top: case CropHandle::Bottom: setCursor(Qt::SizeVerCursor); break;
    case CropHandle::Left: case CropHandle::Right: setCursor(Qt::SizeHorCursor); break;
    case CropHandle::TopLeft: case CropHandle::BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case CropHandle::TopRight: case CropHandle::BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    default: setCursor(Qt::CrossCursor); break;
    }
}

void VideoWidget::updateProtectedCursor(ProtectedHandle handle)
{
    switch(handle) {
    case ProtectedHandle::Move: setCursor(Qt::SizeAllCursor); break;
    case ProtectedHandle::Top: case ProtectedHandle::Bottom: setCursor(Qt::SizeVerCursor); break;
    case ProtectedHandle::Left: case ProtectedHandle::Right: setCursor(Qt::SizeHorCursor); break;
    case ProtectedHandle::TopLeft: case ProtectedHandle::BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case ProtectedHandle::TopRight: case ProtectedHandle::BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    default: setCursor(Qt::CrossCursor); break;
    }
}

void VideoWidget::updateCropFromPointer(const QPointF& p)
{
    QRectF r=m_dragStartRect; const double dx=p.x()-m_dragStartPoint.x(), dy=p.y()-m_dragStartPoint.y();
    switch(m_activeHandle) {
    case CropHandle::Move: r.moveLeft(qBound(0.0,r.left()+dx,1.0-r.width())); r.moveTop(qBound(0.0,r.top()+dy,1.0-r.height())); break;
    case CropHandle::TopLeft: r.setLeft(qBound(0.0,r.left()+dx,r.right()-kMinSize)); r.setTop(qBound(0.0,r.top()+dy,r.bottom()-kMinSize)); break;
    case CropHandle::Top: r.setTop(qBound(0.0,r.top()+dy,r.bottom()-kMinSize)); break;
    case CropHandle::TopRight: r.setRight(qBound(r.left()+kMinSize,r.right()+dx,1.0)); r.setTop(qBound(0.0,r.top()+dy,r.bottom()-kMinSize)); break;
    case CropHandle::Right: r.setRight(qBound(r.left()+kMinSize,r.right()+dx,1.0)); break;
    case CropHandle::BottomRight: r.setRight(qBound(r.left()+kMinSize,r.right()+dx,1.0)); r.setBottom(qBound(r.top()+kMinSize,r.bottom()+dy,1.0)); break;
    case CropHandle::Bottom: r.setBottom(qBound(r.top()+kMinSize,r.bottom()+dy,1.0)); break;
    case CropHandle::BottomLeft: r.setLeft(qBound(0.0,r.left()+dx,r.right()-kMinSize)); r.setBottom(qBound(r.top()+kMinSize,r.bottom()+dy,1.0)); break;
    case CropHandle::Left: r.setLeft(qBound(0.0,r.left()+dx,r.right()-kMinSize)); break;
    default: return;
    }
    m_cropRect=r.normalized(); emit cropChanged(m_cropRect); update();
}

void VideoWidget::updateProtectedFromPointer(const QPointF& p)
{
    QRectF r=m_protectedDragStartRect; const double dx=p.x()-m_protectedDragStartPoint.x(), dy=p.y()-m_protectedDragStartPoint.y();
    switch(m_activeProtectedHandle) {
    case ProtectedHandle::Move: r.moveLeft(qBound(0.0,r.left()+dx,1.0-r.width())); r.moveTop(qBound(0.0,r.top()+dy,1.0-r.height())); break;
    case ProtectedHandle::TopLeft: r.setLeft(qBound(0.0,r.left()+dx,r.right()-kMinSize)); r.setTop(qBound(0.0,r.top()+dy,r.bottom()-kMinSize)); break;
    case ProtectedHandle::Top: r.setTop(qBound(0.0,r.top()+dy,r.bottom()-kMinSize)); break;
    case ProtectedHandle::TopRight: r.setRight(qBound(r.left()+kMinSize,r.right()+dx,1.0)); r.setTop(qBound(0.0,r.top()+dy,r.bottom()-kMinSize)); break;
    case ProtectedHandle::Right: r.setRight(qBound(r.left()+kMinSize,r.right()+dx,1.0)); break;
    case ProtectedHandle::BottomRight: r.setRight(qBound(r.left()+kMinSize,r.right()+dx,1.0)); r.setBottom(qBound(r.top()+kMinSize,r.bottom()+dy,1.0)); break;
    case ProtectedHandle::Bottom: r.setBottom(qBound(r.top()+kMinSize,r.bottom()+dy,1.0)); break;
    case ProtectedHandle::BottomLeft: r.setLeft(qBound(0.0,r.left()+dx,r.right()-kMinSize)); r.setBottom(qBound(r.top()+kMinSize,r.bottom()+dy,1.0)); break;
    case ProtectedHandle::Left: r.setLeft(qBound(0.0,r.left()+dx,r.right()-kMinSize)); break;
    default: return;
    }
    m_protectedRect=r.normalized(); emit protectedRectChanged(m_protectedRect); update();
}

void VideoWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event); QPainter painter(this); painter.setRenderHint(QPainter::SmoothPixmapTransform,true); painter.fillRect(rect(),QColor(16,19,24));
    if(!m_processedFrame.isNull()) { const QSize scaled=m_processedFrame.size().scaled(size(),Qt::KeepAspectRatio); const QRect target((width()-scaled.width())/2,(height()-scaled.height())/2,scaled.width(),scaled.height()); painter.drawImage(target,m_processedFrame); }
    if(m_cropMode) {
        painter.setRenderHint(QPainter::Antialiasing,true); const QRectF r(m_cropRect.left()*width(),m_cropRect.top()*height(),m_cropRect.width()*width(),m_cropRect.height()*height());
        painter.fillRect(rect(),QColor(0,0,0,105)); painter.setCompositionMode(QPainter::CompositionMode_Clear); painter.fillRect(r,Qt::transparent); painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setPen(QPen(Qt::white,2)); painter.setBrush(Qt::white); painter.drawRect(r);
        for(const QPointF& p:{r.topLeft(),r.topRight(),r.bottomRight(),r.bottomLeft()}) painter.drawRect(QRectF(p.x()-kHandle/2,p.y()-kHandle/2,kHandle,kHandle));
    }
    if(m_protectedMode) {
        painter.setRenderHint(QPainter::Antialiasing,true);
        if(m_protectedRect.isNull()) {
            painter.setPen(QPen(QColor(255,190,70),2,Qt::DashLine)); painter.drawText(rect().center(),tr("拖动鼠标创建保护区域"));
        } else {
            const QRectF r(m_protectedRect.left()*width(),m_protectedRect.top()*height(),m_protectedRect.width()*width(),m_protectedRect.height()*height());
            painter.setPen(QPen(QColor(255,190,70),2,Qt::DashLine)); painter.setBrush(Qt::NoBrush); painter.drawRect(r);
            painter.setBrush(QColor(255,190,70));
            for(const QPointF& p:{r.topLeft(),r.topRight(),r.bottomRight(),r.bottomLeft()}) painter.drawRect(QRectF(p.x()-kHandle/2,p.y()-kHandle/2,kHandle,kHandle));
            painter.setPen(QPen(QColor(255,220,150),1)); painter.drawText(r.topLeft()+QPointF(8,18),tr("保护区域"));
        }
    }
}

void VideoWidget::mousePressEvent(QMouseEvent* event)
{
    if(m_protectedMode && event->button()==Qt::LeftButton) {
        const auto hit=hitTestProtected(event->position());
        if(hit==ProtectedHandle::None) { m_activeProtectedHandle=ProtectedHandle::None; m_draggingProtected=true; m_protectedStart=toNormalized(event->position()); m_protectedRect=QRectF(m_protectedStart,m_protectedStart); }
        else { m_activeProtectedHandle=hit; m_draggingProtected=true; m_protectedDragStartRect=m_protectedRect; m_protectedDragStartPoint=toNormalized(event->position()); }
        updateProtectedCursor(hit); update(); return;
    }
    if(!m_cropMode || event->button()!=Qt::LeftButton) { QWidget::mousePressEvent(event); return; }
    const CropHandle hit=hitTest(event->position());
    if(hit==CropHandle::None) { m_activeHandle=CropHandle::None; m_draggingCrop=true; m_cropStart=toNormalized(event->position()); m_cropRect=QRectF(m_cropStart,m_cropStart); }
    else { m_activeHandle=hit; m_draggingCrop=true; m_dragStartRect=m_cropRect; m_dragStartPoint=toNormalized(event->position()); }
    updateCursor(hit); update();
}

void VideoWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(m_protectedMode) {
        const QPointF n=toNormalized(event->position());
        if(!m_draggingProtected) { updateProtectedCursor(hitTestProtected(event->position())); return; }
        if(m_activeProtectedHandle==ProtectedHandle::None) { m_protectedRect=QRectF(m_protectedStart,n).normalized(); m_protectedRect.setLeft(qBound(0.0,m_protectedRect.left(),1.0)); m_protectedRect.setTop(qBound(0.0,m_protectedRect.top(),1.0)); m_protectedRect.setRight(qBound(0.0,m_protectedRect.right(),1.0)); m_protectedRect.setBottom(qBound(0.0,m_protectedRect.bottom(),1.0)); emit protectedRectChanged(m_protectedRect); update(); return; }
        updateProtectedFromPointer(n); return;
    }
    if(!m_cropMode) { QWidget::mouseMoveEvent(event); return; }
    const QPointF normalized=toNormalized(event->position());
    if(!m_draggingCrop) { updateCursor(hitTest(event->position())); return; }
    if(m_activeHandle==CropHandle::None) { m_cropRect=QRectF(m_cropStart,normalized).normalized(); m_cropRect.setLeft(qBound(0.0,m_cropRect.left(),1.0)); m_cropRect.setTop(qBound(0.0,m_cropRect.top(),1.0)); m_cropRect.setRight(qBound(0.0,m_cropRect.right(),1.0)); m_cropRect.setBottom(qBound(0.0,m_cropRect.bottom(),1.0)); emit cropChanged(m_cropRect); update(); return; }
    updateCropFromPointer(normalized);
}

void VideoWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(m_protectedMode && event->button()==Qt::LeftButton) { m_draggingProtected=false; emit protectedRectChanged(m_protectedRect); emit protectedRectCommitted(m_protectedRect); updateProtectedCursor(hitTestProtected(event->position())); update(); return; }
    if(!m_cropMode || event->button()!=Qt::LeftButton) { QWidget::mouseReleaseEvent(event); return; }
    m_draggingCrop=false; emit cropChanged(m_cropRect); emit cropCommitted(m_cropRect); updateCursor(hitTest(event->position())); update();
}
