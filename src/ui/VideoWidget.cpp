#include "VideoWidget.h"

VideoWidget::VideoWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(640, 360);
    setStyleSheet("background:#101318;");
}
