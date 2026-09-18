#pragma once

#include <QWidget>

class VideoWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget* parent = nullptr);

    WId nativeVideoId() const { return winId(); }
};
