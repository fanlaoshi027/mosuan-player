#pragma once

#include <QObject>
#include <QRectF>

class SmartInvertController final : public QObject
{
    Q_OBJECT
public:
    explicit SmartInvertController(QObject* parent = nullptr);

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

    void setProtectedRect(const QRectF& rect);
    QRectF protectedRect() const { return m_protectedRect; }

    void setProtectedBrightness(int brightness);
    int protectedBrightness() const { return m_protectedBrightness; }

signals:
    void changed();

private:
    bool m_enabled = false;
    QRectF m_protectedRect{0.0, 0.0, 0.0, 0.0};
    int m_protectedBrightness = 100;
};
