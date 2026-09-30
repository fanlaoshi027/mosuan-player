#pragma once

#include <QObject>
#include <QString>

class MediaErrorReporter final : public QObject
{
    Q_OBJECT
public:
    explicit MediaErrorReporter(QObject* parent = nullptr);

    void reportOpenFailure(const QString& path, const QString& reason);
    QString lastError() const { return m_lastError; }

signals:
    void errorReported(const QString& message);

private:
    QString m_lastError;
};
