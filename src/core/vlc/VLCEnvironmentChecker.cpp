#include "VLCEnvironmentChecker.h"

#include <QDir>
#include <QFileInfo>

VLCEnvironmentReport VLCEnvironmentChecker::check(const QString& applicationPath)
{
    VLCEnvironmentReport report;

    QFileInfo appInfo(applicationPath);
    report.basePath = appInfo.absolutePath();
    report.executableFound = appInfo.exists();

    QDir base(report.basePath);
    const QStringList candidates = {
        base.filePath("vlc/plugins"),
        base.filePath("plugins")
    };

    for (const QString& path : candidates) {
        if (QDir(path).exists()) {
            report.pluginDirectoryFound = true;
            report.pluginPath = path;
            break;
        }
    }

    if (!report.executableFound)
        report.messages << "application executable not found";

    if (!report.pluginDirectoryFound)
        report.messages << "VLC plugin directory not found";

    return report;
}
