#pragma once

#include <QString>

class NetworkMediaSource final
{
public:
    enum class Protocol {
        Unknown,
        LocalFile,
        SMB
    };

    static Protocol protocolFor(const QString& source);
    static bool isSMB(const QString& source);
    static QString normalizeSMBUrl(const QString& source);
};
