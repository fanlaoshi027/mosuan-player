#pragma once

#include <QString>

struct VLCEnvironmentReport
{
    bool executableFound = false;
    bool pluginDirectoryFound = false;
    QString basePath;
    QString pluginPath;
    QStringList messages;
};

class VLCEnvironmentChecker
{
public:
    static VLCEnvironmentReport check(const QString& applicationPath);
};
