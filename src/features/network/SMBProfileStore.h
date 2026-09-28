#pragma once

#include "SMBProfile.h"
#include <QList>
#include <QString>

class SMBProfileStore final
{
public:
    explicit SMBProfileStore(QString storagePath = {});

    QList<SMBProfile> load() const;
    bool save(const QList<SMBProfile>& profiles) const;
    bool upsert(const SMBProfile& profile) const;
    bool remove(const QString& name) const;

private:
    QString m_storagePath;
};
